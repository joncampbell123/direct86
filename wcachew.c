// memory window

#include "global.h"
#include <stdio.h>
#include "resource.h"
#include "cpuexec.h"
#include "direct86.h"
#include "wcachew.h"

char *szClassNameWCW =		"JMC_PROG_SOTHERE_DEDITx86_WC";
char						wrbuffer2[1024];
char						wrbuffer2b[1024];
int							WCWindowRect_w,WCWindowRect_h;
int							WCRecnt=0;
BOOL						WCRepaint=FALSE;
HWND						hwndMainWC;
HDC							WCWinDC;

long FAR PASCAL MainWndProcWC(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int x,y,h,d,a,t,memaddr,w;
	int bb;
	RECT hwndRec;

	switch (iMessage) {
	
	case WM_CREATE:
		WCWinDC = GetDC(hwnd);
		w = 12*14;
		MoveWindow(hwnd,GetPrivateProfileInt("WCWin","X",140,inifile),GetPrivateProfileInt("WCWin","Y",0,inifile),w,GetPrivateProfileInt("WCWin","H",GetSystemMetrics(SM_CYSCREEN)-120,inifile),TRUE);
		SelectObject(WCWinDC,EditFont);
		SetTextColor(WCWinDC,RGB(0,0,0));
		SetBkColor(WCWinDC,RGB(255,255,255));
		SetTimer(hwnd,1,0,NULL);
		break;

	case WM_SIZE:
		GetClientRect(hwnd,&hwndRec);
		WCWindowRect_w = ((int)hwndRec.right)/efontw;
		WCWindowRect_h = ((int)hwndRec.bottom)/efonth;
		SendMessage(hwnd,WM_TIMER,0,0L);
		break;

	case WM_KEYDOWN:
		a = (int)wParam;
		break;

	case WM_PAINT:
		SendMessage(hwnd,WM_TIMER,0,0L);
		ValidateRect(hwnd,NULL);
		WCRepaint=TRUE;
		WCRecnt=0;
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_CLOSE:
// don't close, just hide yourself
		SendMessage(hwndMain,WM_COMMAND,IDM_WINDOWS_CPUCACHE,0);
		break;

	case WM_LBUTTONDOWN:
		y = HIWORD(lParam);
		y /= 12;
		break;

	case WM_TIMER:
		t = (int)wParam;

		if (WCRecnt > 0 && code_freerun)	WCRecnt--;
		else								WCRecnt=0;

		if ((t == 1 && !WCRepaint) || WCRecnt > 0) return 0L;
		WCRepaint=FALSE;
		WCRecnt=32;

		if (t > 1) return 0L;

		memset(wrbuffer2,32,1023);
		SelectObject(WCWinDC,EditFont);
		SetTextColor(WCWinDC,RGB(0,0,0));
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
		d = WCWindowRect_w - 9;
		d /= 4;
		h = d*3;
		a = 0;
		for (y=0;y <= WCWindowRect_h;y++) {
			sprintf(wrbuffer2,"XXXXXXXX ");
			for (x=0;x < d;x++) {
				bb = -1;

				memaddr = a + x;
				if (bb >= 0)
					sprintf(wrbuffer2b,"%02X ",bb);
				else
					sprintf(wrbuffer2b,"XX ",bb);

				strcat(wrbuffer2,wrbuffer2b);

				if (bb >= 0)
					wrbuffer2[9 + h + x] = bb;
				else
					wrbuffer2[9 + h + x] = '.';
			}
// since the window background is not automatically painted,
// fill the window all the way to the end
			TextOut(WCWinDC,0,y*efonth,wrbuffer2,WCWindowRect_w + 1);
			a += d;
		}
		break;

	case WM_DESTROY:
		GetWindowRect(hwnd,&hwndRec);
		wsprintf(mmtbuf2,"%d",hwndRec.left); WritePrivateProfileString("WCWin","X",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.top); WritePrivateProfileString("WCWin","Y",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.right-hwndRec.left); WritePrivateProfileString("WCWin","W",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.bottom-hwndRec.top); WritePrivateProfileString("WCWin","H",mmtbuf2,inifile);
		PostQuitMessage(0);
		ReleaseDC(hwnd,WCWinDC);
		break;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}
	return 0L;
}
