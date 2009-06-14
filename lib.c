
#include "global.h"
#include <stdio.h>
#include <math.h>
#include "resource.h"
#include "direct86.h"
#include "lib.h"

char*						titleptr;
int							InputUse=0;
BOOL						InputHex=FALSE;
DWORD						InputValue=0;
char						inputbuf[32];
char						inputbufs[1024];

void WriteStatus(char *s)
{
	if (!s)					SetWindowText(hwndMain,szAppName);
	else if (!strlen(s))	SetWindowText(hwndMain,szAppName);
	else					SetWindowText(hwndMain,s);
}

void memrev(char *r,int n)
{
	int o,c;
	char e;
	if (n <= 1) return;

	c=n/2;
	for (o=0;o < c;o++) {
		e=r[n-1-o];
		r[n-1-o] = r[o];
		r[o]=e;
	}
}

void IdleAndESC()			// call to monitor ESC key
{
	MSG msg;

	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {		// keyboard messages only
		if (msg.message == WM_KEYUP) {			// pressed key down?
			if (msg.wParam == VK_ESCAPE) {			// and ESC key?
				UserAbort = TRUE;
			}
		}
	}
}

BOOL CALLBACK DlgAbout(HWND hwnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	int wID;

	if (iMessage == WM_INITDIALOG) {
		SetDlgItemText(hwnd,IDC_ABOUT_VER,version_string);
		SetDlgItemText(hwnd,IDC_ABOUT_COPYRIGHT,copyright_string);
		SetFocus(GetDlgItem(hwnd,IDOK));
	}
	else if (iMessage == WM_COMMAND) {
		wID = (int)wParam;
		if (wID == IDOK) EndDialog(hwnd,0);
	}
	return FALSE;
}

void AboutDlg()
{
#ifdef WIN95
	DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hwndMain,(int (__stdcall *)(struct HWND__ *,unsigned int,unsigned int,long))DlgAbout);
#else
	DialogBox(hInst,MAKEINTRESOURCE(IDD_ABOUT),hwndMain,(int (__stdcall *)())DlgAbout);
#endif
	SetFocus(hwndMain);
}

BOOL CALLBACK DlgInput(HWND hwnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	int wID;

	if (iMessage == WM_INITDIALOG) {
		if (InputHex)	sprintf(inputbuf,"0x%X",InputValue);
		else			sprintf(inputbuf,"%u",InputValue);
		SetDlgItemText(hwnd,IDC_INPUT,inputbuf);
		SetDlgItemText(hwnd,IDC_INPUT_TITLE,titleptr);
		SetFocus(GetDlgItem(hwnd,IDC_INPUT));
	}
	else if (iMessage == WM_COMMAND) {
		wID = (int)wParam;
		if (wID == IDOK) {
			GetDlgItemText(hwnd,IDC_INPUT,inputbuf,31);
			InputValue = strtoul(inputbuf,NULL,0);
			EndDialog(hwnd,0);
		}
	}
	return FALSE;
}

DWORD GetInputFromUser(DWORD init,char *title)
{
	if (InputUse) return 0;

	InputUse=TRUE;
	titleptr = title;
	InputValue = init;
	InputHex = FALSE;
#ifdef WIN95
	DialogBox(hInst,MAKEINTRESOURCE(IDD_INPUTBOX),hwndMain,(int (__stdcall *)(struct HWND__ *,unsigned int,unsigned int,long))DlgInput);
#else
	DialogBox(hInst,MAKEINTRESOURCE(IDD_INPUTBOX),hwndMain,(int (__stdcall *)())DlgInput);
#endif
	SetFocus(hwndMain);
	InputUse=FALSE;

	return InputValue;
}

DWORD GetHexInputFromUser(DWORD init,char *title)
{
	if (InputUse) return 0;

	InputUse=TRUE;
	titleptr = title;
	InputValue = init;
	InputHex = TRUE;
#ifdef WIN95
	DialogBox(hInst,MAKEINTRESOURCE(IDD_INPUTBOX),hwndMain,(int (__stdcall *)(struct HWND__ *,unsigned int,unsigned int,long))DlgInput);
#else
	DialogBox(hInst,MAKEINTRESOURCE(IDD_INPUTBOX),hwndMain,(int (__stdcall *)())DlgInput);
#endif
	SetFocus(hwndMain);
	InputUse=FALSE;
	return InputValue;
}

BOOL CALLBACK DlgInputStr(HWND hwnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	int wID;

	if (iMessage == WM_INITDIALOG) {
		SetFocus(GetDlgItem(hwnd,IDOK));
		SetDlgItemText(hwnd,IDC_INPUT,inputbufs);
	}
	else if (iMessage == WM_COMMAND) {
		wID = (int)wParam;
		if (wID == IDOK) {
			GetDlgItemText(hwnd,IDC_INPUT,inputbufs,1023);
			EndDialog(hwnd,0);
		}
	}
	return FALSE;
}

void GetStringFromUser(char *s)
{
	strcpy(inputbufs,s);
#ifdef WIN95
	DialogBox(hInst,MAKEINTRESOURCE(IDD_INPUTBOX),hwndMain,(int (__stdcall *)(struct HWND__ *,unsigned int,unsigned int,long))DlgInputStr);
#else
	DialogBox(hInst,MAKEINTRESOURCE(IDD_INPUTBOX),hwndMain,(int (__stdcall *)())DlgInputStr);
#endif
	SetFocus(hwndMain);
}

DWORD DecToPackedBCD(DWORD i)
{
	DWORD o;

	o = (i%10);
	i /= 10;
	o |= (i%10)<<4;
	i /= 10;
	o |= (i%10)<<8;
	i /= 10;
	o |= (i%10)<<12;
	i /= 10;
	o |= (i%10)<<16;
	i /= 10;
	o |= (i%10)<<20;
	i /= 10;
	o |= (i%10)<<24;
	i /= 10;
	o |= (i%10)<<28;

	return o;
}

// the following routines calculate the # of seconds relative to
// Jan 1st, 1980. The floating-point numbers allow for milliseconds
double frtime(SYSTEMTIME *st)
{
	double r;
	SYSTEMTIME g;
	int y;
	BOOL leap;

	if (st) {
		memcpy(&g,st,sizeof(SYSTEMTIME));
	}
	else {
		GetSystemTime(&g);
	}

	if (g.wYear < 1980)
		g.wYear = 1980;

	r = 0;
	for (y=1980;y < g.wYear;y++) {
		leap = ((g.wYear & 3) == 0);
		if (leap) {
			if ((g.wYear % 400) == 0) leap=0;
		}

		if (leap)	r += 366.0;
		else		r += 365.0;
	}

	leap = ((g.wYear & 3) == 0);
	if (leap) {
		if ((g.wYear % 400) == 0) leap=0;
	}

	r += (g.wMonth >  1)?31:0;
	r += (g.wMonth >  2)?(leap ? 29 : 28):0;
	r += (g.wMonth >  3)?31:0;
	r += (g.wMonth >  4)?30:0;
	r += (g.wMonth >  5)?31:0;
	r += (g.wMonth >  6)?30:0;
	r += (g.wMonth >  7)?31:0;
	r += (g.wMonth >  8)?31:0;
	r += (g.wMonth >  9)?30:0;
	r += (g.wMonth > 10)?31:0;
	r += (g.wMonth > 11)?30:0;
	r += g.wDay-1;
	r *= 24.0;
	r += g.wHour;
	r *= 60.0;
	r += g.wMinute;
	r *= 60.0;
	r += g.wSecond;
	r *= 1000.0;
	r += g.wMilliseconds;
	r /= 1000.0;
	return r;
}

SYSTEMTIME timefr(double tm)
{
	SYSTEMTIME tt;
	DWORD tday,tmonth,dtm,dtmy,dtmy2,y,ym,yc;

	dtm = (DWORD)floor(tm);
	tt.wMilliseconds = ((WORD)floor(tm * 1000))%1000;
	tt.wSecond = (WORD)(dtm%60); dtm /= 60;
	tt.wMinute = (WORD)(dtm%60); dtm /= 60;
	tt.wHour = (WORD)(dtm%24); dtm /= 24;

	tt.wDayOfWeek = (WORD)((dtm + 2) % 7);		// Jan 1st, 1980 apparently is a Tuesday

	dtmy = dtm;
	dtmy2 = dtmy;
	y = 1980;
	yc=0;
	ym=366;
	while (dtmy2 >= ym) {
		y++;				// increment year
		if (dtmy2 >= ym)	// subtract # of days in year from count
			dtmy2 -= ym;
		else
			dtmy2 = 0;

		yc++;				// cycle through leap years
		if (yc > 3) yc=0;
		if (yc == 0 && y != 2000)	ym=366;	// 2000 is apparently not a leap year
		else						ym=365;
	}

	tt.wYear = (WORD)y;

	tday = (WORD)dtmy2;

	tmonth = 1;
	if (tday >= 31) {			// Past January?
		tday -= 31;
		tmonth++;
	}
// Past February?
	if (tmonth >= 2) {
		if ((tt.wYear & 3) == 0 && (tt.wYear % 400) != 0) {
			if (tday >= 29) {
				tday -= 29;
				tmonth++;
			}
		}
		else {
			if (tday >= 28) {
				tday -= 28;
				tmonth++;
			}
		}
	}
	if (tmonth >= 3) {
		if (tday >= 31) {			// Past March?
			tday -= 31;
			tmonth++;
		}
	}
	if (tmonth >= 4) {
		if (tday >= 30) {			// Past April?
			tday -= 30;
			tmonth++;
		}
	}
	if (tmonth >= 5) {
		if (tday >= 31) {			// Past May?
			tday -= 31;
			tmonth++;
		}
	}
	if (tmonth >= 6) {
		if (tday >= 30) {			// Past June?
			tday -= 30;
			tmonth++;
		}
	}
	if (tmonth >= 7) {
		if (tday >= 31) {			// Past July?
			tday -= 31;
			tmonth++;
		}
	}
	if (tmonth >= 8) {
		if (tday >= 31) {			// Past August?
			tday -= 31;
			tmonth++;
		}
	}
	if (tmonth >= 9) {
		if (tday >= 30) {			// Past September?
			tday -= 30;
			tmonth++;
		}
	}
	if (tmonth >= 10) {
		if (tday >= 31) {			// Past October?
			tday -= 31;
			tmonth++;
		}
	}
	if (tmonth >= 11) {
		if (tday >= 30) {			// Past November?
			tday -= 30;
			tmonth++;
		}
	}
	if (tmonth >= 12) {
		if (tday >= 31) {			// Past December? (a calculation check)
			MessageBox(hwndMain,"tday is too large","timefr() debug message",MB_OK);
		}
	}
	tday++;
	tt.wDay = (WORD)tday;
	tt.wMonth = (WORD)tmonth;

	return tt;
}

int GetListIndex(HWND hwnd,int Lid)
{
	HWND hw;
	int f;

	hw = GetDlgItem(hwnd,Lid);
	f=SendMessage(hw,LB_GETCURSEL,0,0L);
	return f;
}

void GetCurrentListItem(HWND hwnd,int Lid,char *s)
{
	HWND hw;
	unsigned long pt;
	int f;

	hw = GetDlgItem(hwnd,Lid);
	pt = (unsigned long) &s[0];
	f=SendMessage(hw,LB_GETCURSEL,0,0L);
	SendMessage(hw,LB_GETTEXT,f,pt);
}

void AddListBoxItem(HWND hwnd,int Lid,char *s)
{
	char		item[500];
	unsigned	uu,l;
	HWND		hw;
	unsigned long pt;
	
	hw = GetDlgItem(hwnd,Lid);
	pt = (unsigned long) &item[0];
	l=strlen(s);
	if (l > 500) l=500;
	for (uu=0;uu < l+1;uu++) item[uu] = s[uu];
	SendMessage(hw,LB_ADDSTRING,0,pt);
}

void SetListSelection(HWND hwnd,int Lid,int sel)
{
	HWND		hw;
	
	hw = GetDlgItem(hwnd,Lid);
	SendMessage(hw,LB_SETCURSEL,sel,0L);
}
