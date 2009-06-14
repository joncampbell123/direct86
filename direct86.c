
// the main stuff itself

#include "global.h"
#include "resource.h"
#include <stdio.h>
#include "addrbus.h"
#include "fdsim.h"
#include "hdsim.h"
#include "bios.h"
#include "brkpts.h"
#include "cmos.h"
#include "cpudec.h"
#include "cpuexec.h"
#include "cpurgwin.h"
#include "cputail.h"
#include "cpustkw.h"
#include "direct86.h"
#include "execops.h"
#include "execrm.h"
#include "flagops.h"
#include "hardware.h"
#include "interrm.h"
#include "kbdwin.h"
#include "keyboard.h"
#include "lib.h"
#include "memory.h"
#include "memwin.h"
#include "mother.h"
#include "pic.h"
#include "ram.h"
#include "stackops.h"
#include "stockvga.h"
#include "timer.h"
#include "wcache.h"
#include "wcachew.h"
#include "naming.h"

// RAM.H
void RamConfig();

// PIC.H
void PICConfig();

// DIRECT86.C
char *version_string =		"Version 2.00. June 14th, 2009.";
char *copyright_string =	"Copyright (C) 1997-2009, Jonathan \"The Great Codeholio\" Campbell";
char *szAppName =			"Direct 86 v1.0";
char *szClassName =			"JMC_PROG_SOTHERE_DEDITx86";
char *inifile =				"dedit86.ini";
char *common_ini_file =		"dedit86.ini";
HINSTANCE					hInst;
HWND						hwndMain;
HMENU						OurMenu;
BOOL						paintDivider=FALSE;
BOOL						showOpcodes=FALSE;
BOOL						showAddress=TRUE;
BOOL						IsMainActive=FALSE;
BOOL						UserAbort=FALSE;
DWORD						memory_ofs_line_phys[256];
DWORD						memory_ofs_line_cs[256];
DWORD						memory_ofs_line_eip[256];
int							memory_ofs_lines=0;
int							ColumnSize=192;	// bytes per column
HFONT						EditFont;
int							efontw,efonth;
int							WindowWidth,WindowHeight;
BOOL						PaintFlag;
int							DividerX=11;
int							swindow_x;
int							curs_x,curs_y;
long						bcolor,fcolor;
long						hilbcolor,hilfcolor;
char						PaintingSourceDump;	// Flag to prevent DrawSourceDump and CloseFil causing an Invalaid Page Fault
char						AmIActive;
BOOL						ctrldown=FALSE;
char						mmtbuf2[1024];
char						Tbuf2[100];

void RepaintSourceDump(BOOL p);

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR CmdLine, int nCmdShow)
{
	MSG			msg;
	WNDCLASS	wndclass;
	int			exec;

	ComputerInit();
	InitNaming();
	LoadSavedNaming();

	hInst = hInstance;

	if (!hPrevInstance) {
		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = MainWndProc;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_APPICON));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szClassName;

		if (!RegisterClass(&wndclass))
			return FALSE;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = MainWndProcRegs;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_REGICON));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szClassName2;

		if (!RegisterClass(&wndclass))
			return FALSE;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = MainWndProcMem;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_MEMICON));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szClassName3;

		if (!RegisterClass(&wndclass))
			return FALSE;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = MainWndProcKeyb;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_EXEICON));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szClassNameKBW;

		if (!RegisterClass(&wndclass))
			return FALSE;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = MainWndProcWC;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_EXEICON));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szClassNameWCW;

		if (!RegisterClass(&wndclass))
			return FALSE;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = MainWndProcVGA;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_EXEICON));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szClassNameVGA;

		if (!RegisterClass(&wndclass))
			return FALSE;

		wndclass.style = CS_HREDRAW | CS_VREDRAW;
		wndclass.lpfnWndProc = MainWndProcPIC;
		wndclass.cbClsExtra = 0;
		wndclass.cbWndExtra = 0;
		wndclass.hInstance = hInstance;
		wndclass.hIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_EXEICON));
		wndclass.hCursor = NULL;
		wndclass.hbrBackground = NULL;
		wndclass.lpszMenuName = NULL;
		wndclass.lpszClassName = szClassNamePIC;

		if (!RegisterClass(&wndclass))
			return FALSE;
	}

	EditFont=CreateFont(12,8,
						0,0,
						FW_MEDIUM,
						FALSE,FALSE,FALSE,
						DEFAULT_CHARSET,
						OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
						DRAFT_QUALITY,FIXED_PITCH | FF_DONTCARE,
						"Terminal");

	if (EditFont == NULL) {
		MessageBox(NULL,"Window could not be created!!","err",MB_OK);
		return FALSE;
	}

	hwndMain = CreateWindow(szClassName, szAppName,
		WS_OVERLAPPEDWINDOW | WS_HSCROLL,
	    CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
	    NULL, NULL, hInstance, NULL);

	hwndMainRegs = CreateWindow(szClassName2, "Registers",
		WS_BORDER | WS_CAPTION | WS_POPUP,
	    CW_USEDEFAULT,
		CW_USEDEFAULT,
		efontw*17 + (GetSystemMetrics(SM_CXBORDER)*2),
		efonth*28 + (GetSystemMetrics(SM_CYBORDER)*2) + 2 + GetSystemMetrics(SM_CYCAPTION),
	    hwndMain, NULL, hInstance, NULL);

	hwndMainMem = CreateWindow(szClassName3, "Simulated memory",
		WS_SIZEBOX | WS_CAPTION | WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
	    CW_USEDEFAULT,
		CW_USEDEFAULT,
		efontw*78 + (GetSystemMetrics(SM_CXFRAME)*2),
		efonth*18 + (GetSystemMetrics(SM_CYFRAME)*2) + 2 + GetSystemMetrics(SM_CYCAPTION),
	    hwndMain, NULL, hInstance, NULL);

	hwndMainKeyb = CreateWindow(szClassNameKBW, "Simulated keyboard",
		WS_BORDER | WS_CAPTION | WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
	    CW_USEDEFAULT,
		CW_USEDEFAULT,
		508 + (GetSystemMetrics(SM_CXBORDER)*2) + 2,
		158 + (GetSystemMetrics(SM_CYBORDER)*2) + 2 + GetSystemMetrics(SM_CYCAPTION) + 1,
	    hwndMain, NULL, hInstance, NULL);

	hwndMainWC = CreateWindow(szClassNameWCW, "CPU write-through cache",
		WS_SIZEBOX | WS_CAPTION | WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
	    CW_USEDEFAULT,
		CW_USEDEFAULT,
		efontw*48 + (GetSystemMetrics(SM_CXFRAME)*2),
		efonth*7 + (GetSystemMetrics(SM_CYFRAME)*2) + 2 + GetSystemMetrics(SM_CYCAPTION),
	    hwndMain, NULL, hInstance, NULL);

	hwndMainVGA = CreateWindow(szClassNameVGA, "VGA display",
		WS_SIZEBOX | WS_CAPTION | WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
	    CW_USEDEFAULT,
		CW_USEDEFAULT,
		20 + (GetSystemMetrics(SM_CXFRAME)*2),
		20 + (GetSystemMetrics(SM_CYFRAME)*2) + 2 + GetSystemMetrics(SM_CYCAPTION),
	    hwndMain, NULL, hInstance, NULL);

	hwndMainPIC = CreateWindow(szClassNamePIC, "PIC status",
		WS_BORDER | WS_CAPTION | WS_POPUP | WS_MINIMIZEBOX | WS_SYSMENU,
	    CW_USEDEFAULT,
		CW_USEDEFAULT,
		32 + (GetSystemMetrics(SM_CXFRAME)*2),
		50 + (GetSystemMetrics(SM_CYFRAME)*2) + 2 + GetSystemMetrics(SM_CYCAPTION),
	    hwndMain, NULL, hInstance, NULL);

	if (!hwndMain) {
		MessageBox(NULL,"Window could not be created!! 1","err",MB_OK);
		return FALSE;
	}

	ShowWindow(hwndMain, nCmdShow);
	UpdateWindow(hwndMain);
	SetFocus(hwndMain);
	exec=1;

	while (exec) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
			exec=GetMessage(&msg, NULL, 0, 0);
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			StepExecute();
		}
	}

	CloseNaming();

	return msg.wParam;
}

extern DWORD				name_assoc_mem_should_be_ofs;
extern DWORD				name_assoc_mem_should_be_seg;
extern int					naming_enabled;
extern int					name_assoc_mem_itm_idx;

void DrawSourceDump(HDC hDC)
{
	int x,y,rws,l,to,gna,tt,rww;
	char buffer[2048],ii[6];
	char commentbuffer[1024];
	DWORD o,oo,_cs,_ip,_ocs,_oip;
	BOOL HiliteLine;

	if (PaintingSourceDump) return;
	if (swindow_x < 0) swindow_x=0;
	PaintingSourceDump=1;
	y=0;

	BreakpointTriggerCease(1);

// force certain regs
	ireg_flags &= 0x003F7FD7;
	ireg_flags |= 0x00000002;

	SetTextColor(hDC,fcolor);
	SetBkColor(hDC,bcolor);
	SetBkMode(hDC,OPAQUE);
	memset(buffer,177,1024);
	if (PaintFlag) {
		y = curs_y;
		rws = 1;
	}
	else {
		y = 0;
		memory_ofs_lines = rws = (WindowHeight/efonth)+1;
	}
	_cs = debug_window_offset_cs&0xFFFF;
	_ip = debug_window_offset_ip;
	tt=1;
	_ocs = _cs;
	_oip = _ip;
	o = (_cs << 4) + _ip;
	rww=0;

	while (rws > 0) {
		HiliteLine = (((ireg_eip & 0xFFFF) == _ip) && ((ireg_cs & 0xFFFF) == _cs)) ? TRUE : FALSE;

		gna=GetSkippedItems(_ocs,_oip,_cs,_ip,&_cs,&_ip);

		memset(buffer,32,ColumnSize);
		gna=GetNamingMemAssocStrPreIns(_cs,_ip,buffer,900);
		if (gna) {
			if (HiliteLine) {
				SetTextColor(hDC,RGB(255,255,0));
				SetBkColor(hDC,RGB(192,192,192) ^ 0xFFFFFFFF);
			}
			else {
				SetTextColor(hDC,RGB(0,0,128));
				SetBkColor(hDC,RGB(192,192,192));
			}

			if (tt && gna >= 2) {
				_cs = debug_window_offset_cs = name_assoc_mem[name_assoc_mem_itm_idx].seg_begin;
				_ip = debug_window_offset_ip = name_assoc_mem[name_assoc_mem_itm_idx].ofs_begin;
			}
			memory_ofs_line_phys[rww] = oo;
			memory_ofs_line_cs[rww] = _cs;
			memory_ofs_line_eip[rww++] = _ip;

			TextOut(hDC,0,y*efonth,buffer+swindow_x,ColumnSize);
			y++;
			rws--;
		}

		tt=0;
		oo = o;
		_ocs = _cs;
		_oip = _ip;

		if (gna < 2) {
			if (HiliteLine) {
				SetTextColor(hDC,0xFFFFFF);
				SetBkColor(hDC,RGB(192,192,192) ^ 0xFFFFFF);
			}
			else {
				SetTextColor(hDC,0);
				SetBkColor(hDC,RGB(192,192,192));
			}

			memset(buffer,32,ColumnSize);
			DecompileBytes(&_cs,&_ip,buffer);
			o += _ip - _oip;
			memory_ofs_line_phys[rww] = oo;
			memory_ofs_line_cs[rww] = _ocs;
			memory_ofs_line_eip[rww++] = _oip;

			x=strlen(buffer);
			if (GetNamingMemAssocStrComments(_ocs,_oip,commentbuffer,1000)) {
				while (x < 30) buffer[x++] = ' ';
				buffer[x++] = ';';
				buffer[x++] = ' ';
				strcpy(buffer+x,commentbuffer);
			}

			if (DividerX < ColumnSize)	TextOut(hDC,DividerX*efontw,y*efonth,buffer+swindow_x,ColumnSize-DividerX);

			if (DividerX) {
				memset(buffer,32,DividerX);

				if (showAddress) {
					sprintf(buffer,"%04X:%04X",_ocs,_oip);
				}
				if (showOpcodes) {
					memset(buffer,0,DividerX);
					to = oo;
					l = ((int)o) - ((int)oo);
					if (l > 16) l = 16;
					BreakpointTriggerCease(1);
					while (l-- > 0) {
						sprintf(ii,"%02Xh ",membytelinear(to++));
						strcat(buffer,ii);
					}
					BreakpointTriggerCease(0);
				}

				TextOut(hDC,0,y*efonth,buffer,DividerX);
			}

			y++;
			rws--;

			memset(buffer,32,ColumnSize);
			gna=GetNamingMemAssocStrPostIns(_ocs,_oip,buffer,900);
			if (gna) {
				if (HiliteLine) {
					SetTextColor(hDC,RGB(255,255,0));
					SetBkColor(hDC,RGB(192,192,192) ^ 0xFFFFFFFF);
				}
				else {
					SetTextColor(hDC,RGB(0,0,128));
					SetBkColor(hDC,RGB(192,192,192));
				}

				memory_ofs_line_phys[rww] = oo;
				memory_ofs_line_cs[rww] = _ocs;
				memory_ofs_line_eip[rww++] = _oip;

				TextOut(hDC,0,y*efonth,buffer+swindow_x,ColumnSize);
				y++;
				rws--;
			}
		}
		else {
			_ip = name_assoc_mem_should_be_ofs;
			_cs = name_assoc_mem_should_be_seg + ((_ip>>16)<<12);
			_ip &= 0xFFFF;
			_cs &= 0xFFFF;
		}
	}
	PaintingSourceDump=0;
	BreakpointTriggerCease(0);
}

void Rescroll()
{
	BOOL	c;

// run the interpreter!
	c=0;
	if ((curs_x-swindow_x) < 0) {
		swindow_x += (curs_x-swindow_x); 
		c |= 1; 
	}
	if ((curs_x-swindow_x) > ((WindowWidth/efontw)-1-DividerX)) {
		swindow_x += ((curs_x-swindow_x)-((WindowWidth/efontw)-1-DividerX)); 
		c |= 1; 
	}
	if (c) {
		if (c & 1) SetScrollPos(hwndMain,SB_HORZ,swindow_x,TRUE);
		RepaintSourceDump(FALSE);
	}

	if (IsMainActive) SetCaretPos((curs_x-swindow_x+DividerX)*efontw,curs_y*efonth);
}

void RepaintSourceDump(BOOL p)
{
	HDC hDC;
	HFONT hof;

	HideCaret(hwndMain);
	hDC = GetDC(hwndMain);
	hof=(HFONT)SelectObject(hDC,EditFont);
	SetMapMode(hDC,MM_TEXT);
	SetBkMode(hDC,OPAQUE);
	PaintFlag=p;
	DrawSourceDump(hDC);
	SendMessage(hwndMainRegs,WM_TIMER,0,0L);
	SelectObject(hDC,hof);
	ReleaseDC(hwndMain,hDC);
	SetScrollPos(hwndMain,SB_HORZ,swindow_x,TRUE);
	ShowCaret(hwndMain);
}

DWORD Decompile_seg;
DWORD Decompile_ofsbeg,Decompile_ofsend;
char Decompile_path[1024];

BOOL CALLBACK DecompileToFileDlgProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	char buf[512];

	if (msg == WM_INITDIALOG) {
		sprintf(buf,"0x%04X",Decompile_seg);
		SetDlgItemText(hwnd,IDC_RDSEG,buf);
		sprintf(buf,"0x%04X",Decompile_ofsbeg);
		SetDlgItemText(hwnd,IDC_FROM,buf);
		sprintf(buf,"0x%04X",Decompile_ofsend);
		SetDlgItemText(hwnd,IDC_TO,buf);
		SetDlgItemText(hwnd,IDC_PATH,Decompile_path);

		return TRUE;
	}
	else if (msg == WM_COMMAND) {
		if (wparam == IDOK) {
			GetDlgItemText(hwnd,IDC_RDSEG,buf,511);
			Decompile_seg = strtoul(buf,NULL,0);
			GetDlgItemText(hwnd,IDC_FROM,buf,511);
			Decompile_ofsbeg = strtoul(buf,NULL,0);
			GetDlgItemText(hwnd,IDC_TO,buf,511);
			Decompile_ofsend = strtoul(buf,NULL,0);
			GetDlgItemText(hwnd,IDC_PATH,Decompile_path,1020);

			EndDialog(hwnd,1);
		}
		if (wparam == IDCANCEL) {
			EndDialog(hwnd,0);
		}
	}

	return FALSE;
}

void DecompileToFileMenu()
{
	GetPrivateProfileString("DecompileToFile","FilePath","c:\\deasm.txt",Decompile_path,1020,common_ini_file);
	if (!DialogBox(hInst,MAKEINTRESOURCE(IDD_DECOMPILE),hwndMain,DecompileToFileDlgProc)) return;
	WritePrivateProfileString("DecompileToFile","FilePath",Decompile_path,common_ini_file);

	DecompileToFile(Decompile_seg,Decompile_ofsbeg,Decompile_ofsend,Decompile_path);
}

long FAR PASCAL MainWndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int wID,spc;
	HDC		hDC;
	HFONT	hof;
	RECT	hwndRec;
	DWORD	or;
	HWND	hwn;
	int		c,r,g,b,x;
	SIZE	sz;

	switch (iMessage) {
	
	case WM_CREATE:
		naming_enabled = GetPrivateProfileInt("Options","NamingEnabled",1,common_ini_file);

		hDC = GetDC(hwnd);
		SetMapMode(hDC,MM_TEXT);
		SelectObject(hDC,EditFont);
		GetTextExtentPoint32(hDC,"a",1,&sz);
		ReleaseDC(hwnd,hDC);
		efontw = sz.cx;
		efonth = sz.cy;
		MoveWindow(hwnd,GetPrivateProfileInt("MainWin","X",140,inifile),GetPrivateProfileInt("MainWin","Y",0,inifile),GetPrivateProfileInt("MainWin","W",GetSystemMetrics(SM_CXSCREEN)-150,inifile),GetPrivateProfileInt("MainWin","H",
			GetSystemMetrics(SM_CYSCREEN)-120,inifile),TRUE);

		OurMenu = LoadMenu(hInst,MAKEINTRESOURCE(IDM_MENU));
		SetMenu(hwnd,OurMenu);
		r = GetPrivateProfileInt("Background","R",800,inifile);
		g = GetPrivateProfileInt("Background","G",800,inifile);
		b = GetPrivateProfileInt("Background","B",800,inifile);
		if (r == 800 || g == 800 || b == 800) {
			WritePrivateProfileString("Background","R","0",inifile);
			WritePrivateProfileString("Background","G","0",inifile);
			WritePrivateProfileString("Background","B","0",inifile);
			r=0;
			g=0;
			b=0;
		}
		bcolor = RGB(r,g,b);
		r = GetPrivateProfileInt("Foreground","R",800,inifile);
		g = GetPrivateProfileInt("Foreground","G",800,inifile);
		b = GetPrivateProfileInt("Foreground","B",800,inifile);
		if (r == 800 || g == 800 || b == 800) {
			WritePrivateProfileString("Foreground","R","255",inifile);
			WritePrivateProfileString("Foreground","G","255",inifile);
			WritePrivateProfileString("Foreground","B","255",inifile);
			r=255;
			g=255;
			b=255;
		}
		fcolor = RGB(r,g,b);
		hilbcolor = bcolor ^ RGB(0,0,255);
		hilfcolor = fcolor;
			
		curs_x=0;
		curs_y=0;
		swindow_x=0;
		SetTimer(hwnd,1,0,NULL);

		if (naming_enabled)		CheckMenuItem(OurMenu,IDC_SYMNAMES_ENABLE,MF_CHECKED | MF_BYCOMMAND);
		else					CheckMenuItem(OurMenu,IDC_SYMNAMES_ENABLE,MF_UNCHECKED | MF_BYCOMMAND);
		break;

	case WM_ACTIVATE:
		wID = LOWORD(wParam);
		if (wID == WA_ACTIVE || wID == WA_CLICKACTIVE) {
			DestroyCaret();
			CreateCaret(hwnd,NULL,efontw,efonth);
			SetCaretPos((curs_x-swindow_x+DividerX)*efontw,curs_y*efonth);
			ShowCaret(hwnd);

			AmIActive=1;
			IsMainActive=TRUE;
		}
		if (wID == WA_INACTIVE) {
			hwn = (HWND)lParam;
			DestroyCaret();
			AmIActive=0;
			IsMainActive=FALSE;
		}
		break;

	case WM_COMMAND:
		wID = LOWORD(wParam);
		if (wID == IDC_VIEW_SYMVARS)		NamingVarsConfig();
		if (wID == IDC_CONFIG_NAMING)		NamingConfig();
		if (wID == IDC_RELOAD_NAMING)		LoadSavedNaming();
		if (wID == IDC_DECOMPILE_TO_FILE)	DecompileToFileMenu();
		if (wID == IDM_MENU_SIM_RESET)		ComputerReset();
		if (wID == IDC_COMPUTER_POWERON)	ComputerPowerOn();
		if (wID == IDC_COMPUTER_POWEROFF)	ComputerPowerOff();
		if (wID == IDM_MENU_FILE_EXIT)		PostMessage(hwnd,WM_DESTROY,0,0L);
		if (wID == IDC_SIMULATION_BREAKS)	UserEditBreakPoints();
		if (wID == IDM_ABOUT)				AboutDlg();
		if (wID == IDM_EDIT_BIOS)			ConfigureBIOS();
		if (wID == IDC_EDIT_CPU)			ConfigureCPU();
		if (wID == IDC_CPUTRAILVIEW)		CPUTrailsView();
		if (wID == IDC_CPUSTACKWVIEW)		StackWatchView();
		if (wID == IDC_MEM_CONFIGRAM)		RamConfig();
		if (wID == IDC_PIC_CONFIG)			PICConfig();
		if (wID == IDM_EDIT_CPUSPEED)		code_cpuspeed = GetInputFromUser(code_cpuspeed,"Speed in instructions/second");
		if (wID == IDC_ADDREMDEV)			ComputerDevicesConfig();
		if (wID == IDC_SYMNAMES_ENABLE) {
			naming_enabled = !naming_enabled;

			if (naming_enabled)		CheckMenuItem(OurMenu,IDC_SYMNAMES_ENABLE,MF_CHECKED | MF_BYCOMMAND);
			else					CheckMenuItem(OurMenu,IDC_SYMNAMES_ENABLE,MF_UNCHECKED | MF_BYCOMMAND);
		}
		if (wID == IDM_WINDOWS_REGS) {
			register_win_showing = !register_win_showing;
			if (register_win_showing) {
				ShowWindow(hwndMainRegs,SW_SHOW);
				CheckMenuItem(OurMenu,IDM_WINDOWS_REGS,MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				ShowWindow(hwndMainRegs,SW_HIDE);
				CheckMenuItem(OurMenu,IDM_WINDOWS_REGS,MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		if (wID == ID_STATUS_MEMORYDUMP) {
			memory_win_showing = !memory_win_showing;
			if (memory_win_showing) {
				ShowWindow(hwndMainMem,SW_SHOW);
				CheckMenuItem(OurMenu,ID_STATUS_MEMORYDUMP,MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				ShowWindow(hwndMainMem,SW_HIDE);
				CheckMenuItem(OurMenu,ID_STATUS_MEMORYDUMP,MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		if (wID == IDM_WINDOWS_KEYB) {
			keyboard_win_showing = !keyboard_win_showing;
			if (keyboard_win_showing) {
				ShowWindow(hwndMainKeyb,SW_SHOW);
				CheckMenuItem(OurMenu,IDM_WINDOWS_KEYB,MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				ShowWindow(hwndMainKeyb,SW_HIDE);
				CheckMenuItem(OurMenu,IDM_WINDOWS_KEYB,MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		if (wID == IDM_WINDOWS_DISPLAY) {
			display_win_showing = !display_win_showing;
			if (display_win_showing) {
				ShowWindow(hwndMainVGA,SW_SHOW);
				CheckMenuItem(OurMenu,IDM_WINDOWS_DISPLAY,MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				ShowWindow(hwndMainVGA,SW_HIDE);
				CheckMenuItem(OurMenu,IDM_WINDOWS_DISPLAY,MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		if (wID == IDM_WINDOWS_CPUCACHE) {
			if (!IsWindowVisible(hwndMainWC)) {
				ShowWindow(hwndMainWC,SW_SHOW);
				CheckMenuItem(OurMenu,IDM_WINDOWS_CPUCACHE,MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				ShowWindow(hwndMainWC,SW_HIDE);
				CheckMenuItem(OurMenu,IDM_WINDOWS_CPUCACHE,MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		if (wID == IDM_WINDOWS_PIC) {
			if (!IsWindowVisible(hwndMainPIC)) {
				ShowWindow(hwndMainPIC,SW_SHOW);
				CheckMenuItem(OurMenu,IDM_WINDOWS_PIC,MF_BYCOMMAND | MF_CHECKED);
			}
			else {
				ShowWindow(hwndMainPIC,SW_HIDE);
				CheckMenuItem(OurMenu,IDM_WINDOWS_PIC,MF_BYCOMMAND | MF_UNCHECKED);
			}
		}
		
		Rescroll();
		break;

	case WM_HSCROLL:
		wID = LOWORD(wParam);
		spc = GetScrollPos(hwnd,SB_HORZ);
		if (wID == SB_BOTTOM)			SetScrollPos(hwnd,SB_HORZ,65535,TRUE);
		if (wID == SB_LINEDOWN)			SetScrollPos(hwnd,SB_HORZ,spc+1,TRUE);
		if (wID == SB_LINEUP)			SetScrollPos(hwnd,SB_HORZ,spc-1,TRUE);
		if (wID == SB_PAGEDOWN)			SetScrollPos(hwnd,SB_HORZ,spc+10,TRUE);
		if (wID == SB_PAGEUP)			SetScrollPos(hwnd,SB_HORZ,spc-10,TRUE);
		if (wID == SB_THUMBTRACK)		SetScrollPos(hwnd,SB_HORZ,HIWORD(wParam),TRUE);
		if (wID == SB_TOP)				SetScrollPos(hwnd,SB_HORZ,0,TRUE);
		spc = GetScrollPos(hwnd,SB_HORZ);
		swindow_x = spc;
		if (curs_x < swindow_x) {
			curs_x = swindow_x;
		}
		if (curs_x > (swindow_x+(WindowWidth/efontw))) {
			curs_x = (swindow_x+(WindowWidth/efontw));
		}
		SetScrollPos(hwndMain,SB_HORZ,swindow_x,TRUE);
		if (IsMainActive) {
			SetCaretPos((curs_x-swindow_x+DividerX)*efontw,curs_y*efonth);
			ShowCaret(hwnd);
		}
		RepaintSourceDump(FALSE);
		break;

	case WM_KEYDOWN:
		wID=(int)wParam;
		c=0;
		if (wID == VK_CONTROL)							ctrldown=TRUE;
		if (wID == VK_DELETE) {
		}
		if (wID == VK_PRIOR) {
			if (curs_y > 0) {
				curs_y = 0;
				RepaintSourceDump(FALSE);
			}
			else {
				for (x=0;x < ((WindowHeight/efonth)-1);x++) {
					SendMessage(hwnd,WM_KEYDOWN,VK_UP,-1);
				}
				RepaintSourceDump(FALSE);
			}
			c=1;
		}
		if (wID == VK_NEXT) {
			if (curs_y < ((WindowHeight/efonth)-1)) {
				curs_y = ((WindowHeight/efonth)-1);
			}
			else {
				debug_window_offset_ip = memory_ofs_line_eip[(WindowHeight/efonth)-1];
				debug_window_offset_cs = memory_ofs_line_cs[(WindowHeight/efonth)-1];
			}
			RepaintSourceDump(FALSE);
			c=1;
		}
		if (ctrldown) {
			if (wID == VK_LEFT && DividerX > 0) {
				DividerX--;
				curs_x++;
				RepaintSourceDump(FALSE);
				c=1;
			}
			if (wID == VK_RIGHT && DividerX < 48) {
				DividerX++;
				curs_x--;
				RepaintSourceDump(FALSE);
				c=1;
			}
		}
		else {
			if (wID == VK_LEFT && curs_x > -DividerX) {
				curs_x--;
				c=1;
			}
			if (wID == VK_RIGHT) {
				if (curs_x < (ColumnSize-1)) {
					curs_x++;
				}
				c=1;
			}
		}
		if (wID == VK_UP) {
			if (curs_y) {
				curs_y--;
			}
			else {
				or = (debug_window_offset_cs << 4) + debug_window_offset_ip;
				if (debug_window_offset_ip) {
					debug_window_offset_ip--;
				}
				else {
					debug_window_offset_ip = 0xFFFF;
					debug_window_offset_cs -= 0x1000;
				}
			}
			if (lParam != -1) RepaintSourceDump(FALSE);
			c=1;
		}
		if (wID == VK_DOWN) {
			if (curs_y < ((WindowHeight/efonth)-1)) {
				curs_y++;
			}
			else {
				if (memory_ofs_line_eip[0] == memory_ofs_line_eip[1]) {
					if (memory_ofs_line_eip[1] == memory_ofs_line_eip[2]) {
						debug_window_offset_ip = memory_ofs_line_eip[3];
						debug_window_offset_cs = memory_ofs_line_cs[3];
						debug_window_offset32 = memory_ofs_line_phys[3];
					}
					else {
						debug_window_offset_ip = memory_ofs_line_eip[2];
						debug_window_offset_cs = memory_ofs_line_cs[2];
						debug_window_offset32 = memory_ofs_line_phys[2];
					}
				}
				else {
					debug_window_offset_ip = memory_ofs_line_eip[1];
					debug_window_offset_cs = memory_ofs_line_cs[1];
					debug_window_offset32 = memory_ofs_line_phys[1];
				}
			}
			RepaintSourceDump(FALSE);
			c=1;
		}
		if (wID == VK_HOME) {
			curs_x=0;
			c=1;
		}
		if (wID == VK_END) {
			c=1;
		}
		if (wID == VK_F8) {
			ExecutionStep();
			ExecutionShow();
		}
		if (wID == VK_F5) {
			code_freerun = !code_freerun;
		}
		if (wID == VK_F6) {
			code_showallsteps = !code_showallsteps;
		}
		if (c) Rescroll();
		if (!c)	return DefWindowProc(hwnd,iMessage,wParam,lParam);
		break;

	case WM_KEYUP:
		wID=(int)wParam;
		if (wID == VK_CONTROL)				ctrldown=FALSE;
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
		break;

	case WM_ERASEBKGND:
		hDC = (HDC)wParam;
		SelectObject(hDC,GetStockObject(WHITE_BRUSH));
		SelectObject(hDC,GetStockObject(NULL_PEN));
		Rectangle(hDC,0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
		break;
	
	case WM_TIMER:
		wID = (int)wParam;
		break;
	
	case WM_CHAR:
		c = (char) wParam;
		if (c == 4) {
			showOpcodes ^= 1;
			RepaintSourceDump(FALSE);
		}
		if (curs_x < 0) {
			if (((tolower(c) >= 'a' && tolower(c) <= 'f') ||
				isdigit(c)) && showAddress) {

				x = curs_x + DividerX;

				debug_window_offset_ip += (DWORD)curs_y;
				if (debug_window_offset_ip > 0xFFFF) {
					debug_window_offset_ip &= 0xFFFF;
					debug_window_offset_cs += 0x1000;
				}
		
				if (x == 0 && isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0x0FFF) | ((c - '0')<<12);
				if (x == 0 && !isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0x0FFF) | ((tolower(c) - 'a' + 10)<<12);
				if (x == 1 && isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0xF0FF) | ((c - '0')<<8);
				if (x == 1 && !isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0xF0FF) | ((tolower(c) - 'a' + 10)<<8);
				if (x == 2 && isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0xFF0F) | ((c - '0')<<4);
				if (x == 2 && !isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0xFF0F) | ((tolower(c) - 'a' + 10)<<4);
				if (x == 3 && isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0xFFF0) | (c - '0');
				if (x == 3 && !isdigit(c))
					debug_window_offset_cs = (debug_window_offset_cs & 0xFFF0) | (tolower(c) - 'a' + 10);
					
				if (x == 5 && isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0x0FFF) | ((c - '0')<<12);
				if (x == 5 && !isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0x0FFF) | ((tolower(c) - 'a' + 10)<<12);
				if (x == 6 && isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0xF0FF) | ((c - '0')<<8);
				if (x == 6 && !isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0xF0FF) | ((tolower(c) - 'a' + 10)<<8);
				if (x == 7 && isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0xFF0F) | ((c - '0')<<4);
				if (x == 7 && !isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0xFF0F) | ((tolower(c) - 'a' + 10)<<4);
				if (x == 8 && isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0xFFF0) | (c - '0');
				if (x == 8 && !isdigit(c))
					debug_window_offset_ip = (debug_window_offset_ip & 0xFFF0) | (tolower(c) - 'a' + 10);

				debug_window_offset_ip -= (DWORD)curs_y;
				if ((int)debug_window_offset_ip < 0) {
					debug_window_offset_ip &= 0xFFFF;
					debug_window_offset_cs -= 0x1000;
				}

				curs_x++;
			}
		}
		Rescroll();
		break;

	case WM_RBUTTONDOWN:
		break;

	case WM_MOUSEMOVE:
		break;

	case WM_RBUTTONUP:
		break;

	case WM_LBUTTONDOWN:
		curs_x=swindow_x+(LOWORD(lParam)/efontw)-DividerX;
		curs_y=(HIWORD(lParam)/efonth);
		if (IsMainActive) {
			HideCaret(hwnd);
			SetCaretPos((curs_x-swindow_x+DividerX)*efontw,curs_y*efonth);
			ShowCaret(hwnd);
		}
		break;

	case WM_PAINT:
		ValidateRect(hwnd,NULL);
		hDC = GetDC(hwnd);
		hof=(HFONT)SelectObject(hDC,EditFont);
		SetMapMode(hDC,MM_TEXT);
		SetBkMode(hDC,OPAQUE);

		PaintFlag=0;
		DrawSourceDump(hDC);

		SelectObject(hDC,hof);
		ReleaseDC(hwnd,hDC);
		SetScrollRange(hwnd,SB_HORZ,0,ColumnSize,TRUE);
		break;

	case WM_SIZE:
		WindowWidth = LOWORD(lParam);
		WindowHeight = HIWORD(lParam);
		break;

	case WM_DESTROY:
		ComputerClose();

		DestroyWindow(hwndMainRegs);
		DestroyWindow(hwndMainMem);
		DestroyWindow(hwndMainKeyb);
		DestroyWindow(hwndMainWC);
		DestroyWindow(hwndMainVGA);
		DestroyWindow(hwndMainPIC);

		GetWindowRect(hwnd,&hwndRec);
		wsprintf(mmtbuf2,"%d",hwndRec.left); WritePrivateProfileString("MainWin","X",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.top); WritePrivateProfileString("MainWin","Y",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.right-hwndRec.left); WritePrivateProfileString("MainWin","W",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.bottom-hwndRec.top); WritePrivateProfileString("MainWin","H",mmtbuf2,inifile);
		PostQuitMessage(0);
		DeleteObject(EditFont);

		wsprintf(mmtbuf2,"%d",naming_enabled); WritePrivateProfileString("Options","NamingEnabled",mmtbuf2,common_ini_file);
		break;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}
	return 0L;
}
