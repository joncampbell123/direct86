// PIC (Programmable Interrupt Controller) simulation code

#include "global.h"
#include <stdio.h>
#include "resource.h"
#include "direct86.h"
#include "cpuexec.h"
#include "brkpts.h"
#include "pic.h"
#include "stackops.h"
#include "addrbus.h"
#include "hardware.h"

int CanInterruptCPU();

pic_controller				pic_1;
pic_controller				pic_2;
char *szClassNamePIC =		"JMC_PROG_SOTHERE_DEDITx86_PIC";
int							interrupt_disp_sig[16];
int							interrupt_serv_sig[16];
HDC							PICWinDC;
int							PICWindowRect_w,PICWindowRect_h;
BOOL						PICRepaint;
static char					wrbuffer2[2048];
HWND						hwndMainPIC;
int							pic_power=0;
// emulation flags
int							pic_at=1;			// AT style PIC
int							pic_irq2chain=1;	// Emulate IRQ2 -> IRQ9 chain
int							pic_atirqprio=1;	// Emulate AT style PIC priorities

// called by other "hardware" in this program to signal
// IRQ 0-15

void HW_Signal_IRQ(int n)
{
	if (n < 0 || n > 15) return;

	if (pic_at) {
// emulation of IRQ2 -> IRQ9 remapping on AT+ PICs
		if (pic_irq2chain && n == 2) n = 9;
	}
	else {
		if (n > 7) return;		// PC/XT computers never had IRQ 8-15
	}

	if (n >= 8)		pic_2.int_pending |= (1<<(n-8));
	else			pic_1.int_pending |= (1<<n);

	PICRepaint=1;
	interrupt_disp_sig[n] = 2;
}

int PIC_Dispatch(pic_controller *pic)
{
	int x,sig,msk,intn,irq2chk;

// set up masks for checking interrupts. calculate based on what
// we are told is the lowest priority interrupt. this way we only
// scan from highest priority to lowest priority.
	msk=0x80>>pic->int_lowest_prio;
	irq2chk=(pic == &pic_1) && pic_at && pic_atirqprio;
	sig=0;
	intn=pic->int_lowest_prio+1; if (intn > 7) intn=0;
	for (x=0;x < 8 && !sig;) {
		if (irq2chk && intn == 2) {
// emulate AT+ style priorities where processing of IRQ2 causes
// the PIC to process IRQ 8 through 15 (but never really processes
// IRQ 2)
			sig=PIC_Dispatch(&pic_2);
		}
		else {
// stop if we encounter an interrupt in service, so only higher
// priority interrupts can occur and lower ones get ignored for
// a moment in favor of higher priority
			if (pic->int_inservice & msk) {
// update var that controls display
				interrupt_serv_sig[intn + pic->irq_base]=2;
				x=8;
			}
			else if ((pic->int_pending & msk) && (pic->int_mask & msk)) {
				pic->int_inservice |= msk;
				pic->int_pending &= ~msk;
				pic->last_int = intn;
				InterruptCPU(pic->int_base + intn);
				sig=1;
				interrupt_serv_sig[intn + pic->irq_base]=2;
			}
		}

		x++;
		msk <<= 1; if (msk & 0x100) msk = 0x01;
		intn++; if (intn > 7) intn = 0;

	}

	return sig;
}

// perform non-specific EOI
void nonspec_EOI(pic_controller *pic)
{
	int x,sig,msk,intn;

// set up masks for checking interrupts. calculate based on what
// we are told is the lowest priority interrupt. this way we only
// scan from highest priority to lowest priority.
	msk=0x80>>pic->int_lowest_prio;
	sig=0;
	intn=pic->int_lowest_prio+1; if (intn > 7) intn=0;
	for (x=0;x < 8;) {
// stop if we encounter an interrupt in service, so only higher
		if (pic->int_inservice & msk) {
			pic->int_inservice &= ~msk;
			x=8;
		}

		x++;
		msk <<= 1; if (msk & 0x100) msk = 0x01;
		intn++; if (intn > 7) intn = 0;
	}
}

// this function is called for every CPU "cycle"
// so the PIC can be simulated even if 
// I/O is not occurring (which is important for proper
// emulation of the programmable interrupt controller)
void PICCycle()
{
	int sig;

	if (!pic_power) return;
	if (!CanInterruptCPU()) return;

// cycle through IRQs and interrupt CPU
	sig=0;
	if (!sig)		sig=PIC_Dispatch(&pic_1);
	if (!sig)		sig=PIC_Dispatch(&pic_2);
}

// PIC command handler
BYTE pic_cmd(pic_controller *pic,BYTE cmd,int dir)
{
	BYTE r;

	r=0;

	if (dir == IO_PORT_OUT) {
		pic->mode = PIC_MODE_NORMAL;

// read interrupt request register
		if (cmd == 0x0A) {
			pic->cmddat_ret = (BYTE)(pic->int_pending&0xFF);
			pic->mode = PIC_MODE_IRRRET;
		}
// read interrupt in-service register
		if (cmd == 0x0B) {
			pic->cmddat_ret = (BYTE)(pic->int_inservice&0xFF);
			pic->mode = PIC_MODE_ISRRET;
		}
// No-Op?
		if (cmd == 0x40) {
		}
// clear special mask mode?
		if (cmd == 0x48) {
		}
// set special mask mode?
		if (cmd == 0x68) {
		}
// non-specific EOI
		if (cmd == 0x20) {
// determine last IRQ based on priority
			nonspec_EOI(pic);
		}
// rotate on non-specific EOI?
		if (cmd == 0xA0) {
			if (pic->last_int >= 0) {
				nonspec_EOI(pic);
				pic->int_lowest_prio = pic->last_int; // switch the one we cleared to lowest priority
				pic->last_int = -1;
				PICRepaint=1;
			}
		}
// specific EOI?
		if ((cmd & 0xF8) == 0x60) {
			pic->int_inservice &= ~(1 << (cmd&7));
			PICRepaint=1;
		}
// IRQ lowest priority?
		if ((cmd & 0xF8) == 0xC0) {
			pic->int_lowest_prio = cmd&7;
			PICRepaint=1;
		}
// EOI and IRQ lowest priority?
		if ((cmd & 0xF8) == 0xE0) {
			pic->int_inservice &= ~(1 << (cmd&7));
			pic->int_lowest_prio = cmd&7;
			PICRepaint=1;
		}
	}

	if (dir == IO_PORT_IN) {
		if (pic->mode == PIC_MODE_NORMAL)
			r = (BYTE)(pic->cmddat_ret&0xFF);

		if (pic->mode == PIC_MODE_ISRRET)
			r = (BYTE)(pic->int_inservice&0xFF);

		if (pic->mode == PIC_MODE_IRRRET)
			r = (BYTE)(pic->int_pending&0xFF);
	}

	return r;
}

// PIC data/interrupt mask handler
BYTE pic_dat(pic_controller *pic,BYTE dat,int dir)
{
	BYTE r;

	r=0;

	if (dir == IO_PORT_OUT) {
		pic->int_mask = (~((int)dat)) & 0xFF;
		PICRepaint=1;
	}

	if (dir == IO_PORT_IN) {
		r = (~pic->int_mask) & 0xFF;
	}

	return r;
}

DWORD pic_20h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)pic_cmd(&pic_1,0,IO_PORT_IN);
	}

	if (dir == IO_PORT_OUT) {
		data &= 0xFF;
		pic_cmd(&pic_1,(BYTE)data,IO_PORT_OUT);
	}

	return r;
}

DWORD pic_21h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)pic_dat(&pic_1,0,IO_PORT_IN);
	}

	if (dir == IO_PORT_OUT) {
		data &= 0xFF;
		pic_dat(&pic_1,(BYTE)data,IO_PORT_OUT);
	}

	return r;
}

DWORD pic_A0h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)pic_cmd(&pic_2,0,IO_PORT_IN);
	}

	if (dir == IO_PORT_OUT) {
		data &= 0xFF;
		pic_cmd(&pic_2,(BYTE)data,IO_PORT_OUT);
	}

	return r;
}

DWORD pic_A1h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)pic_dat(&pic_2,0,IO_PORT_IN);
	}

	if (dir == IO_PORT_OUT) {
		data &= 0xFF;
		pic_dat(&pic_2,(BYTE)data,IO_PORT_OUT);
	}

	return r;
}

void PIC_UnassignRes()
{
	unassign_IO_port(0x20);
	unassign_IO_port(0x21);
	if (pic_at) {
		unassign_IO_port(0xA0);
		unassign_IO_port(0xA1);
	}
}

void PIC_AssignRes()
{
	assign_IO_port(0x20,pic_20h);
	assign_IO_port(0x21,pic_21h);
	if (pic_at) {
		assign_IO_port(0xA0,pic_A0h);
		assign_IO_port(0xA1,pic_A1h);
	}
}

void InitPIC()
{
	pic_power=0;
}

void ClosePIC()
{
	pic_power=0;
	PIC_UnassignRes();
}

void PICReset()
{
	int i;

	if (!pic_power) return;

// reset to IBM PC compatible settings

// primary interrupt controller
	pic_1.base_io = 0x20;
	pic_1.int_base = 0x08;
	pic_1.int_mask = 0xFF;
	pic_1.int_pending = 0x00;
	pic_1.int_inservice = 0x00;
	pic_1.int_lowest_prio = 7;
	pic_1.last_int = -1;
	pic_1.cmddat_ret = 0x00;
	pic_1.mode = 0;
	pic_1.irq_base = 0;

// secondary interrupt controller
	pic_2.base_io = 0xA0;
	pic_2.int_base = 0x70;
	pic_2.int_mask = 0xFF;
	pic_2.int_pending = 0x00;
	pic_2.int_inservice = 0x00;
	pic_2.int_lowest_prio = 7;
	pic_2.last_int = -1;
	pic_2.cmddat_ret = 0x00;
	pic_2.mode = 0;
	pic_2.irq_base = 8;

// assign ourself to PIC I/O ports
	PIC_AssignRes();

	PICRepaint=1;
	pic_power=1;

	for (i=0;i < 16;i++) {
		interrupt_serv_sig[i]=0;
		interrupt_disp_sig[i]=0;
	}
}

void PICPowerOn()
{
	pic_power=1;
	PICReset();
}

void PICPowerOff()
{
	pic_power=0;
	ClosePIC();
}

int PICIsPowered()
{
	return pic_power;
}

long FAR PASCAL MainWndProcPIC(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int y,h,d,a,t,w,i,bb;
	RECT hwndRec;

	switch (iMessage) {
	
	case WM_CREATE:
		PICWinDC = GetDC(hwnd);
		w = 32*efontw;
		h = 19*efonth;
#ifdef WIN95
		w += (GetSystemMetrics(SM_CXEDGE)*2);
		h += (GetSystemMetrics(SM_CYEDGE)*2) + GetSystemMetrics(SM_CYCAPTION);
#else
		w += (GetSystemMetrics(SM_CXBORDER)*2);
		h += (GetSystemMetrics(SM_CYBORDER)*2) + GetSystemMetrics(SM_CYCAPTION);
#endif
		MoveWindow(hwnd,GetPrivateProfileInt("PICWin","X",140,inifile),GetPrivateProfileInt("PICWin","Y",0,inifile),w,h,TRUE);
		SelectObject(PICWinDC,EditFont);
		SetTextColor(PICWinDC,RGB(0,0,0));
		SetBkColor(PICWinDC,RGB(255,255,255));
		SetTimer(hwnd,1,0,NULL);
		break;

	case WM_SIZE:
		GetClientRect(hwnd,&hwndRec);
		PICWindowRect_w = ((int)hwndRec.right)/efontw;
		PICWindowRect_h = ((int)hwndRec.bottom)/efonth;
		SendMessage(hwnd,WM_TIMER,0,0L);
		break;

	case WM_KEYDOWN:
		a = (int)wParam;
		break;

	case WM_PAINT:
		ReleaseDC(hwnd,PICWinDC);
		PICWinDC = GetDC(hwnd);
		SendMessage(hwnd,WM_TIMER,0,0L);
		ValidateRect(hwnd,NULL);
		PICRepaint=TRUE;
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_CLOSE:
// don't close, just hide yourself
		SendMessage(hwndMain,WM_COMMAND,IDM_WINDOWS_PIC,0);
		break;

	case WM_LBUTTONDOWN:
		y = HIWORD(lParam);
		y /= efonth;

		if (y >= 1 && y <= 16) {
			bb = y-1;

			if (bb >= 0 && bb < 16) {
				pic_controller *pic;

				if (bb >= 8) {
					pic = &pic_2;
					i = bb-8;
				}
				else {
					pic = &pic_1;
					i = bb;
				}

				pic->int_mask ^= (1<<i);
				PICRepaint=1;
			}
		}
		break;

	case WM_TIMER:
		t = (int)wParam;

		for (i=0;i < 8;i++) {
			if (pic_1.int_inservice & (1<<i)) {
				if (!interrupt_serv_sig[i+pic_1.irq_base]) PICRepaint=1;
				interrupt_serv_sig[i+pic_1.irq_base]=2;
			}
			if (pic_2.int_inservice & (1<<i)) {
				if (!interrupt_serv_sig[i+pic_2.irq_base]) PICRepaint=1;
				interrupt_serv_sig[i+pic_2.irq_base]=2;
			}
		}

		for (i=0;i < 16;i++) {
			if (interrupt_disp_sig[i] > 0) {
				interrupt_disp_sig[i]--;

				if (interrupt_disp_sig[i] == 0) PICRepaint=1;
			}

			if (interrupt_serv_sig[i] > 0) {
				interrupt_serv_sig[i]--;

				if (interrupt_serv_sig[i] == 0) PICRepaint=1;
			}
		}

		if (!PICRepaint) return 0L;
		if (t > 1) return 0L;
		PICRepaint=FALSE;

		memset(wrbuffer2,32,1023);
		SelectObject(PICWinDC,EditFont);
		SetTextColor(PICWinDC,RGB(0,0,0));
// solve the system
// t = total memory window width
// h = width of hex dump partition
// d = width of ascii dump partition
// /--
// | t = h + d + 14
// < h = 3x
// | d = x
// \--
// first, substitute and calculcate as follows:
// t = 3x + x + 14, simplififed down to
// t = 4x + 14
// and solve for X
		d = PICWindowRect_w - 9;
		d /= 4;
		h = d*3;
		a = 0;
		for (y=0;y < 19;y++) {
			bb = y-1;

			if (bb >= 0 && bb < 16) {
				pic_controller *pic;
				char *lowpriority_str[] = {"      ","LOWEST"};
				char *mask_str[] = {" OFF"," ON "};
				char *sig_str[] = {"   ","SIG"};
				char *serv_str[] = {"    ","SERV"};

				if (bb >= 8) {
					pic = &pic_2;
					i = bb-8;
				}
				else {
					pic = &pic_1;
					i = bb;
				}

				sprintf(wrbuffer2,"IRQ %2u: %02Xh %s %s %s %s",i + pic->irq_base,
					i + pic->int_base,
					mask_str[(pic->int_mask >> i)&1],
					sig_str[interrupt_disp_sig[bb]?1:0],
					serv_str[interrupt_serv_sig[bb]?1:0],
					lowpriority_str[(pic->int_lowest_prio == i)?1:0]);
			}
			else {
				memset(wrbuffer2,0,PICWindowRect_w + 1);
			}
// since the window background is not automatically painted,
// fill the window all the way to the end
			TextOut(PICWinDC,0,y*efonth,wrbuffer2,PICWindowRect_w + 1);
			a += d;
		}
		break;

	case WM_DESTROY:
		GetWindowRect(hwnd,&hwndRec);
		wsprintf(mmtbuf2,"%d",hwndRec.left); WritePrivateProfileString("PICWin","X",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.top); WritePrivateProfileString("PICWin","Y",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.right-hwndRec.left); WritePrivateProfileString("PICWin","W",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.bottom-hwndRec.top); WritePrivateProfileString("PICWin","H",mmtbuf2,inifile);
		PostQuitMessage(0);
		ReleaseDC(hwnd,PICWinDC);
		break;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}
	return 0L;
}

BOOL CALLBACK PICConfigDlg(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	if (msg == WM_INITDIALOG) {
		if (pic_at)		CheckDlgButton(hwnd,IDC_AT_PIC,TRUE);
		else			CheckDlgButton(hwnd,IDC_XT_PIC,TRUE);

		CheckDlgButton(hwnd,IDC_AT_IRQ2REMAP,pic_irq2chain);
		CheckDlgButton(hwnd,IDC_AT_IRQPRIO,pic_atirqprio);

		return TRUE;
	}
	if (msg == WM_COMMAND) {
		if (wParam == IDOK) {
			PIC_UnassignRes();
			pic_at=IsDlgButtonChecked(hwnd,IDC_AT_PIC);
			pic_irq2chain=IsDlgButtonChecked(hwnd,IDC_AT_IRQ2REMAP);
			pic_atirqprio=IsDlgButtonChecked(hwnd,IDC_AT_IRQPRIO);
			PIC_AssignRes();

			EndDialog(hwnd,1);
		}
		if (wParam == IDCANCEL) {
			EndDialog(hwnd,0);
		}
	}

	return FALSE;
}

void PICConfig()
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_PICCONFIG),hwndMain,PICConfigDlg);
	SetFocus(hwndMain);
}
