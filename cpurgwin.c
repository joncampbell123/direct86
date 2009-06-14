// register window code

#include "global.h"
#include "direct86.h"
#include "cpurgwin.h"
#include "cpuexec.h"
#include "lib.h"
#include <stdio.h>

char *szClassName2 =		"JMC_PROG_SOTHERE_DEDITx86_REGS";
char						rbuffer[1024];
HWND						hwndMainRegs;
HDC							RegWinDC;
BOOL						register_win_showing=FALSE;

void SetupRegWndDC()
{
	SelectObject(RegWinDC,EditFont);
	SetTextColor(RegWinDC,RGB(0,0,0));
	SetBkColor(RegWinDC,RGB(255,255,255));
}

long FAR PASCAL MainWndProcRegs(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int x,y;
	LONG tmp;

	switch (iMessage) {
	
	case WM_CREATE:
		RegWinDC = GetDC(hwnd);
		SetupRegWndDC();
		break;

	case WM_PAINT:
		ReleaseDC(hwnd,RegWinDC);
		RegWinDC = GetDC(hwnd);
		SetupRegWndDC();
		SendMessage(hwnd,WM_TIMER,0,0L);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_LBUTTONDOWN:
		x = LOWORD(lParam)/efontw;
		y = HIWORD(lParam)/efonth;

		if (y == 1)
			ireg_eax = GetHexInputFromUser(ireg_eax,"Value of EAX");
		if (y == 2)
			ireg_ebx = GetHexInputFromUser(ireg_ebx,"Value of EBX");
		if (y == 3)
			ireg_ecx = GetHexInputFromUser(ireg_ecx,"Value of ECX");
		if (y == 4)
			ireg_edx = GetHexInputFromUser(ireg_edx,"Value of EDX");
		if (y == 5)
			ireg_esi = GetHexInputFromUser(ireg_esi,"Value of ESI");
		if (y == 6)
			ireg_edi = GetHexInputFromUser(ireg_edi,"Value of EDI");
		if (y == 7)
			ireg_esp = GetHexInputFromUser(ireg_esp,"Value of ESP");
		if (y == 8)
			ireg_ebp = GetHexInputFromUser(ireg_ebp,"Value of EBP");
		if (y == 9)
			ireg_eip = GetHexInputFromUser(ireg_eip,"Value of EIP");
		if (y == 10)
			ireg_flags = GetHexInputFromUser(ireg_flags,"Value of FLAGS");
		if (y == 12)
			ireg_cs = GetHexInputFromUser(ireg_cs,"Value of CS");
		if (y == 13)
			ireg_ds = GetHexInputFromUser(ireg_ds,"Value of DS");
		if (y == 14)
			ireg_es = GetHexInputFromUser(ireg_es,"Value of ES");
		if (y == 15)
			ireg_fs = GetHexInputFromUser(ireg_fs,"Value of FS");
		if (y == 16)
			ireg_gs = GetHexInputFromUser(ireg_gs,"Value of GS");
		if (y == 17)
			ireg_ss = GetHexInputFromUser(ireg_ss,"Value of SS");
		if (y == 18) {
			tmp = ireg_idtr;
			ireg_idtr = (LONG)(GetHexInputFromUser((DWORD)(tmp & 0xFFFF),"IDT limit")&0xFFFF);
			ireg_idtr |= ((LONG)GetHexInputFromUser((DWORD)(tmp>>16),"IDT address")<<16);
		}
		if (y == 19) {
			tmp = ireg_gdtr;
			ireg_gdtr = (LONG)(GetHexInputFromUser((DWORD)(tmp & 0xFFFF),"GDT limit")&0xFFFF);
			ireg_gdtr |= ((LONG)GetHexInputFromUser((DWORD)(tmp>>16),"GDT address")<<16);
		}
		if (y == 21) {
			if (x == 1 || x == 2)
				ireg_flags ^= 1;
			if (x == 4 || x == 5)
				ireg_flags ^= 0x40;
			if (x == 7 || x == 8)
				ireg_flags ^= 0x200;
			if (x == 10)
				ireg_flags ^= 0x80;
			if (x == 12 || x == 13)
				ireg_flags ^= 0x10;
		}
		if (y == 22) {
			if (x == 1 || x == 2)
				ireg_flags ^= 4;
			if (x == 4 || x == 5)
				ireg_flags ^= 0x400;
			if (x == 7 || x == 8)
				ireg_flags ^= 0x800;
		}
		if (y == 24) {
			if (x <= 8) {
				if (ireg_datasegreset) {
					if (ireg_dataseg == &ireg_cs)
						ireg_dataseg = &ireg_ds;
					else if (ireg_dataseg == &ireg_ds)
						ireg_dataseg = &ireg_es;
					else if (ireg_dataseg == &ireg_es)
						ireg_dataseg = &ireg_fs;
					else if (ireg_dataseg == &ireg_fs)
						ireg_dataseg = &ireg_gs;
					else if (ireg_dataseg == &ireg_gs)
						ireg_dataseg = &ireg_ss;
					else
						ireg_datasegreset = 0;
				}
				else {
					ireg_datasegreset = 1;
					ireg_dataseg = &ireg_cs;
				}
			}
			else if (x >= 10 && x <= 13) {
				exec_buslock = !exec_buslock;
			}
		}
		if (y == 26) {
			ireg_cr0 = GetHexInputFromUser(ireg_cr0,"Value of CR0");
		}

		SendMessage(hwnd,WM_TIMER,0,0L);
		break;
	
	case WM_TIMER:
		SetTextColor(RegWinDC,RGB(0,0,0));

		sprintf(rbuffer,"EAX = %08X",ireg_eax);
		TextOut(RegWinDC,efontw,efonth,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"EBX = %08X",ireg_ebx);
		TextOut(RegWinDC,efontw,efonth*2,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"ECX = %08X",ireg_ecx);
		TextOut(RegWinDC,efontw,efonth*3,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"EDX = %08X",ireg_edx);
		TextOut(RegWinDC,efontw,efonth*4,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"ESI = %08X",ireg_esi);
		TextOut(RegWinDC,efontw,efonth*5,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"EDI = %08X",ireg_edi);
		TextOut(RegWinDC,efontw,efonth*6,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"ESP = %08X",ireg_esp);
		TextOut(RegWinDC,efontw,efonth*7,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"EBP = %08X",ireg_ebp);
		TextOut(RegWinDC,efontw,efonth*8,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"EIP = %08X",ireg_eip);
		TextOut(RegWinDC,efontw,efonth*9,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"EFL = %08X",ireg_flags);
		TextOut(RegWinDC,efontw,efonth*10,rbuffer,strlen(rbuffer));

		sprintf(rbuffer,"CS = %04X",ireg_cs&0xFFFF);
		TextOut(RegWinDC,efontw,efonth*12,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"DS = %04X",ireg_ds&0xFFFF);
		TextOut(RegWinDC,efontw,efonth*13,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"ES = %04X",ireg_es&0xFFFF);
		TextOut(RegWinDC,efontw,efonth*14,rbuffer,strlen(rbuffer));
		if (cpu_revision >= 3)		SetTextColor(RegWinDC,RGB(0,0,0));
		else						SetTextColor(RegWinDC,RGB(192,192,192));
		sprintf(rbuffer,"FS = %04X",ireg_fs&0xFFFF);
		TextOut(RegWinDC,efontw,efonth*15,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"GS = %04X",ireg_gs&0xFFFF);
		TextOut(RegWinDC,efontw,efonth*16,rbuffer,strlen(rbuffer));
		SetTextColor(RegWinDC,RGB(0,0,0));
		sprintf(rbuffer,"SS = %04X",ireg_ss&0xFFFF);
		TextOut(RegWinDC,efontw,efonth*17,rbuffer,strlen(rbuffer));
		if (cpu_revision >= 3)		SetTextColor(RegWinDC,RGB(0,0,0));
		else						SetTextColor(RegWinDC,RGB(192,192,192));
		sprintf(rbuffer,"IDTR%012lX",ireg_idtr);
		TextOut(RegWinDC,0,efonth*18,rbuffer,strlen(rbuffer));
		sprintf(rbuffer,"GDTR%012lX",ireg_gdtr);
		TextOut(RegWinDC,0,efonth*19,rbuffer,strlen(rbuffer));

		SetTextColor(RegWinDC,(ireg_flags & 0x0001) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw,efonth*21,"CF",2);
		SetTextColor(RegWinDC,(ireg_flags & 0x0040) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw*4,efonth*21,"ZR",2);
		SetTextColor(RegWinDC,(ireg_flags & 0x0200) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw*7,efonth*21,"IE",2);
		SetTextColor(RegWinDC,(ireg_flags & 0x0080) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw*10,efonth*21,"S",1);
		SetTextColor(RegWinDC,(ireg_flags & 0x0010) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw*12,efonth*21,"AC",2);
		SetTextColor(RegWinDC,(ireg_flags & 0x0004) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw,efonth*22,"PE",2);
		SetTextColor(RegWinDC,(ireg_flags & 0x0400) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw*4,efonth*22,"DF",2);
		SetTextColor(RegWinDC,(ireg_flags & 0x0800) ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw*7,efonth*22,"OF",2);

// segment override info

		SetTextColor(RegWinDC,RGB(0,0,0));
		if (ireg_datasegreset) {
			strcpy(rbuffer,"Seg = ");
			if (ireg_dataseg == &ireg_cs)
				strcat(rbuffer,"CS");
			else if (ireg_dataseg == &ireg_ds)
				strcat(rbuffer,"DS");
			else if (ireg_dataseg == &ireg_es)
				strcat(rbuffer,"ES");
			else if (ireg_dataseg == &ireg_fs)
				strcat(rbuffer,"FS");
			else if (ireg_dataseg == &ireg_gs)
				strcat(rbuffer,"GS");
			else if (ireg_dataseg == &ireg_ss)
				strcat(rbuffer,"SS");
			else
				strcat(rbuffer,"??");
		}
		else {
			memset(rbuffer,32,256);
		}
		TextOut(RegWinDC,efontw*1,efonth*24,rbuffer,8);

// LOCK info

		SetTextColor(RegWinDC,exec_buslock ? RGB(0,0,0) : RGB(192,192,192));
		TextOut(RegWinDC,efontw*10,efonth*24,"LOCK",4);

// control registers

		if (cpu_revision >= 3)		SetTextColor(RegWinDC,RGB(0,0,0));
		else						SetTextColor(RegWinDC,RGB(192,192,192));
		sprintf(rbuffer,"CR0 = %08X",ireg_cr0);
		TextOut(RegWinDC,efontw,efonth*26,rbuffer,strlen(rbuffer));
		break;

	case WM_DESTROY:
		ReleaseDC(hwnd,RegWinDC);
		break;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

	case WM_ACTIVATE:
		ReleaseDC(hwnd,RegWinDC);
		RegWinDC = GetDC(hwnd);
		SetupRegWndDC();
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}
	return 0L;
}
