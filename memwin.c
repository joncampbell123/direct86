// memory window

#include "global.h"
#include "resource.h"
#include <stdio.h>
#include "brkpts.h"
#include "direct86.h"
#include "cpurgwin.h"
#include "addrbus.h"
#include "cpuexec.h"
#include "memwin.h"

BYTE snapshothardmembyte(DWORD o);

char *szClassName3 =		"JMC_PROG_SOTHERE_DEDITx86_MEM";
char						rbuffer2[1024];
char						rbuffer2b[256];
BOOL						MemoryWinUpdate=FALSE;
DWORD						memory_win_begin,memory_win_end;
DWORD						memory_window_cs=0;
DWORD						memory_window_eip=0;
int							MemWindowRect_w=0,MemWindowRect_h=0;
int							cursor_mem_x=0,cursor_mem_y=0;
HWND						hwndMainMem;
HDC							MemWinDC;
BOOL						memory_win_showing=FALSE;

long FAR PASCAL MainWndProcMem(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int x,y,h,d,a,t,l,mb,spaddr,memaddr;
	RECT hwndRec;

	switch (iMessage) {
	
	case WM_CREATE:
		MemWinDC = GetDC(hwnd);
		MoveWindow(hwnd,GetPrivateProfileInt("MemWin","X",140,inifile),GetPrivateProfileInt("MemWin","Y",0,inifile),GetPrivateProfileInt("MemWin","W",GetSystemMetrics(SM_CXSCREEN)-150,inifile),GetPrivateProfileInt("MemWin","H",
			GetSystemMetrics(SM_CYSCREEN)-120,inifile),TRUE);

		SelectObject(RegWinDC,EditFont);
		SetTextColor(RegWinDC,RGB(0,0,0));
		SetBkColor(RegWinDC,RGB(255,255,255));
		cursor_mem_x=cursor_mem_y=0;
		SetTimer(hwnd,1,0,NULL);
		break;

	case WM_ACTIVATE:
		ReleaseDC(hwnd,MemWinDC);
		MemWinDC = GetDC(hwnd);

		x = LOWORD(wParam);
		if (x == WA_ACTIVE || WA_CLICKACTIVE) {
			DestroyCaret();

			CreateCaret(hwnd,NULL,efontw,efonth);

			SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
			ShowCaret(hwnd);
			AmIActive=1;
		}
		if (x == WA_INACTIVE) {
			DestroyCaret();
			AmIActive=0;
		}
		break;

	case WM_SIZE:
		MemWindowRect_w = LOWORD(lParam) / efontw;
		MemWindowRect_h = HIWORD(lParam) / efonth;
		break;

	case WM_CHAR:
		x = (char)wParam;
		BreakpointTriggerCease(1);
		if (x == 9) {
// calculate the needed variables
			d = MemWindowRect_w - 14;
			d /= 4;
			h = d*3;
			if (cursor_mem_x >= 0 && cursor_mem_x <= 4) {
				cursor_mem_x = 4;
				SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0L);
			}
			else if (cursor_mem_x >= 5 && cursor_mem_x <= 13) {
				cursor_mem_x = 13;
				SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0L);
			}
			else if (cursor_mem_x >= 14 && cursor_mem_x <= (13+h)) {
				cursor_mem_x = 13+h;
				SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0L);
			}
			else if (cursor_mem_x >= (14+h) && cursor_mem_x <= (13+h+d)) {
				cursor_mem_x = 0;
				SendMessage(hwnd,WM_KEYDOWN,VK_DOWN,0L);
			}
		}
		if (x >= 32) {
// calculate the needed variables
			d = MemWindowRect_w - 14;
			d /= 4;
			h = d*3;
// check the cursor's position and act accordingly
// are we typing over the adress bar (segment portion)
// here?
			if (cursor_mem_x >= 0 && cursor_mem_x <= 3) {
				t = (3 - cursor_mem_x)*4;
				memory_window_cs &= ~(0x0F << t);
				if (a = isdigit(x))
					memory_window_cs |= (x - '0')<<t;
				else if (a = isxdigit(x))
					memory_window_cs |= (tolower(x) - 'a' + 10)<<t;

				if (cursor_mem_x == 3)
					SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0L);

				if (a)
					SendMessage(hwnd,WM_TIMER,0,0L);
			}
// what about the offset part of the address bar?
			if (cursor_mem_x >= 5 && cursor_mem_x <= 12) {
				t = (12 - cursor_mem_x)*4;
				memory_window_eip &= ~(0x0F << t);
				if (a = isdigit(x))
					memory_window_eip |= (x - '0')<<t;
				else if (a = isxdigit(x))
					memory_window_eip |= (tolower(x) - 'a' + 10)<<t;

				if (cursor_mem_x == 12)
					SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0L);

				if (a)
					SendMessage(hwnd,WM_TIMER,0,0L);
			}
// or is the cursor within the hex dump?
			if (cursor_mem_x >= 14 && cursor_mem_x < (14+h)) {
				l = ((cursor_mem_x - 14)%3);

				if (l < 2) {
					t = memory_window_eip + ((cursor_mem_x - 14) / 3) + (memory_window_cs*16) + (cursor_mem_y * d);

					mb = hardmembyte(t);

					if (a = isdigit(x))
						hardwritemembyte(t,(BYTE)((mb & ~(0x0F << ((1 - l) * 4))) | ((x - '0') << ((1 - l) * 4))));
					else if (a = isxdigit(x))
						hardwritemembyte(t,(BYTE)((mb & ~(0x0F << ((1 - l) * 4))) | ((tolower(x) - 'a' + 10) << ((1 - l) * 4))));
				}
				if (l == 1) {
					if (cursor_mem_x >= (12+h)) {
						cursor_mem_x = 13;
						SendMessage(hwnd,WM_KEYDOWN,VK_DOWN,0L);
					}
					else {
						SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0L);
					}
				}

				if (a)
					SendMessage(hwnd,WM_TIMER,0,0L);
			}
// perhaps the ASCII dump?
			if (cursor_mem_x >= (14+h) && cursor_mem_x < (14+h+d)) {
				t = memory_window_eip + (cursor_mem_x - 14 - h) + (memory_window_cs*16) + (cursor_mem_y * d);

				hardwritemembyte(t,(BYTE)x);

				if (cursor_mem_x >= (13+h+d)) {
					cursor_mem_x = 13+h;
					SendMessage(hwnd,WM_KEYDOWN,VK_DOWN,0L);
				}

				if (a)
					SendMessage(hwnd,WM_TIMER,0,0L);
			}
// move cursor
			SendMessage(hwnd,WM_KEYDOWN,VK_RIGHT,0L);
		}
		else if (x == 13) {
			SendMessage(hwnd,WM_KEYDOWN,VK_HOME,0L);
			SendMessage(hwnd,WM_KEYDOWN,VK_DOWN,0L);
		}
		BreakpointTriggerCease(0);
		break;

	case WM_KEYDOWN:
		a = (int)wParam;
		d = MemWindowRect_w - 14;
		d /= 4;
		if (a == VK_UP) {
			if (cursor_mem_y) {
				cursor_mem_y--;
				SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
			}
			else {
				memory_window_eip -= d;
				SendMessage(hwnd,WM_TIMER,0,0L);
			}
		}
		if (a == VK_HOME && cursor_mem_x) {
			cursor_mem_x = 0;
			SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
		}
		if (a == VK_END && cursor_mem_x < (MemWindowRect_w-1)) {
			cursor_mem_x = MemWindowRect_w-1;
			SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
		}
		if (a == VK_PRIOR) {
			if (cursor_mem_y) {
				cursor_mem_y=0;
				SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
			}
			else {
				memory_window_eip -= d*MemWindowRect_h;
				SendMessage(hwnd,WM_TIMER,0,0L);
			}
		}
		if (a == VK_DOWN) {
			if (cursor_mem_y < (MemWindowRect_h - 1)) {
				cursor_mem_y++;
				SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
			}
			else {
				memory_window_eip += d;
				SendMessage(hwnd,WM_TIMER,0,0L);
			}
		}
		if (a == VK_NEXT) {
			if (cursor_mem_y < (MemWindowRect_h - 1)) {
				cursor_mem_y = MemWindowRect_h - 1;
				SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
			}
			else {
				memory_window_eip += d*MemWindowRect_h;
				SendMessage(hwnd,WM_TIMER,0,0L);
			}
		}
		if (a == VK_LEFT && cursor_mem_x) {
			cursor_mem_x--;
			SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
		}
		if (a == VK_RIGHT && cursor_mem_x < (MemWindowRect_w-1)) {
			cursor_mem_x++;
			SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
		}
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_LBUTTONUP:
		HideCaret(hwnd);
		x = LOWORD(lParam) / efontw;
		y = HIWORD(lParam) / efonth;
		cursor_mem_x = x;
		cursor_mem_y = y;
		SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
		ShowCaret(hwnd);
		break;

	case WM_PAINT:
		ReleaseDC(hwnd,MemWinDC);
		MemWinDC = GetDC(hwnd);
		SelectObject(RegWinDC,EditFont);
		SetTextColor(RegWinDC,RGB(0,0,0));
		SetBkColor(RegWinDC,RGB(255,255,255));

		SendMessage(hwnd,WM_TIMER,0,0L);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_CLOSE:
// don't close, just hide yourself
		SendMessage(hwndMain,WM_COMMAND,ID_STATUS_MEMORYDUMP,0);
		break;
	
	case WM_TIMER:
		t = (int)wParam;

		if (t == 1 && !MemoryWinUpdate) return 0L;
		MemoryWinUpdate=FALSE;

		if (t > 1) return 0L;

		BreakpointTriggerCease(1);
		memset(rbuffer2,32,1023);
		SelectObject(MemWinDC,EditFont);
		SetTextColor(MemWinDC,RGB(0,0,0));
		HideCaret(hwnd);
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
		d = MemWindowRect_w - 14;
		d /= 4;
		h = d*3;
		a = 0;
		memory_win_begin = (memory_window_cs * 16) + memory_window_eip;
		spaddr = ((ireg_ss&0xFFFF)<<4) + (ireg_esp&0xFFFF);
		for (y=0;y <= MemWindowRect_h;y++) {
			sprintf(rbuffer2,"%04X:%08X ",memory_window_cs,memory_window_eip + a);
			for (x=0;x < d;x++) {
				memaddr = (memory_window_cs * 16) + memory_window_eip + a + x;
				sprintf(rbuffer2b,"%02X ",snapshothardmembyte(memaddr));
				if (spaddr == memaddr)
					rbuffer2b[2] = 's';
				
				strcat(rbuffer2,rbuffer2b);
			}
			for (x=0;x < d;x++) {
				rbuffer2[14 + h + x] = hardmembyte((memory_window_cs * 16) + memory_window_eip + a + x);
			}
// since the window background is not automatically painted,
// fill the window all the way to the end
			TextOut(MemWinDC,0,y*efonth,rbuffer2,MemWindowRect_w + 1);
			a += d;
		}
		memory_win_end = (memory_window_cs * 16) + memory_window_eip + a + d;
		SetCaretPos(cursor_mem_x*efontw,cursor_mem_y*efonth);
		ShowCaret(hwnd);
		BreakpointTriggerCease(0);
		break;

	case WM_DESTROY:
		GetWindowRect(hwnd,&hwndRec);
		wsprintf(mmtbuf2,"%d",hwndRec.left); WritePrivateProfileString("MemWin","X",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.top); WritePrivateProfileString("MemWin","Y",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.right-hwndRec.left); WritePrivateProfileString("MemWin","W",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.bottom-hwndRec.top); WritePrivateProfileString("MemWin","H",mmtbuf2,inifile);
		PostQuitMessage(0);
		ReleaseDC(hwnd,MemWinDC);
		break;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}
	return 0L;
}
