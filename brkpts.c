// debugging breakpoint management

#include "global.h"
#include "brkpts.h"
#include "direct86.h"
#include "cpuexec.h"
#include "resource.h"
#include <stdio.h>

char*						breakpoint_en[2] = {"x"," "};
int							new_breakpoint_idx;
HFONT						UserEditBreakDlgFont=NULL;
BREAKPOINT					breakpoints[MAX_BREAKPOINTS];
int							hold_breakpoint_triggers=0;
int							exec_show_breakpoint=0;
int							breakpoint_powered=0;

#define BREAKPOINT_TYPE_MAX			9
char *breakpoint_typestr[BREAKPOINT_TYPE_MAX] = {
	"<none>",
	"Memory",
	"I/O port",
	"S/W interrupt",
	"H/W interrupt",
};

void BreakpointTriggerCease(int i)
{
	hold_breakpoint_triggers=i;
}

int DoesTriggerMemBreakpoint(DWORD addr,DWORD siz,int access)
{
	int i,f;

	if (!breakpoint_powered)		return -1;
	if (breakpoints[0].type <= 0)	return -1;
	if (hold_breakpoint_triggers)	return -1;

	i=f=0;
	while (i < MAX_BREAKPOINTS && !f) {
		if (breakpoints[i].type == BPT_MEMRANGE) {
			if (breakpoints[i].enabled) {
				if (((breakpoints[i].trigger & BPTR_READ) && (access & BPTR_READ)) ||
					((breakpoints[i].trigger & BPTR_WRITE) && (access & BPTR_WRITE))) {

					if ((addr+siz) > breakpoints[i].mem_addr && addr <= breakpoints[i].mem_addr_end) {
						f=1;
					}
				}
			}
		}
		else if (breakpoints[i].type <= 0) {
			i=MAX_BREAKPOINTS;
		}

		if (!f) i++;
	}

	if (!f) return -1;

	return i;
}

int DoesTriggerIOBreakpoint(DWORD addr,DWORD siz,int access)
{
	int i,f;

	if (!breakpoint_powered)		return -1;
	if (breakpoints[0].type <= 0)	return -1;
	if (hold_breakpoint_triggers)	return -1;

	i=f=0;
	while (i < MAX_BREAKPOINTS && !f) {
		if (breakpoints[i].type == BPT_IOPORTRANGE) {
			if (breakpoints[i].enabled) {
				if (((breakpoints[i].trigger & BPTR_READ) && (access & BPTR_READ)) ||
					((breakpoints[i].trigger & BPTR_WRITE) && (access & BPTR_WRITE))) {

					if ((addr+siz) > breakpoints[i].io_addr && addr <= breakpoints[i].io_addr_end) {
						f=1;
					}
				}
			}
		}
		else if (breakpoints[i].type <= 0) {
			i=MAX_BREAKPOINTS;
		}

		if (!f) i++;
	}

	if (!f) return -1;

	return i;
}

int DoesTriggerSWINTBreakpoint(int j)
{
	int i,f;

	if (!breakpoint_powered)		return -1;
	if (breakpoints[0].type <= 0)	return -1;
	if (hold_breakpoint_triggers)	return -1;

	i=f=0;
	while (i < MAX_BREAKPOINTS && !f) {
		if (breakpoints[i].type == BPT_SWINTRANGE) {
			if (breakpoints[i].enabled) {
				if (j >= breakpoints[i].swint_addr && j <= breakpoints[i].swint_addr_end) {
					f=1;
				}
			}
		}
		else if (breakpoints[i].type <= 0) {
			i=MAX_BREAKPOINTS;
		}

		if (!f) i++;
	}

	if (!f) return -1;

	return i;
}

int DoesTriggerHWINTBreakpoint(int j)
{
	int i,f;

	if (!breakpoint_powered)		return -1;
	if (breakpoints[0].type <= 0)	return -1;
	if (hold_breakpoint_triggers)	return -1;

	i=f=0;
	while (i < MAX_BREAKPOINTS && !f) {
		if (breakpoints[i].type == BPT_HWINTRANGE) {
			if (breakpoints[i].enabled) {
				if (j >= breakpoints[i].hwint_addr && j <= breakpoints[i].hwint_addr_end) {
					f=1;
				}
			}
		}
		else if (breakpoints[i].type <= 0) {
			i=MAX_BREAKPOINTS;
		}

		if (!f) i++;
	}

	if (!f) return -1;

	return i;
}

void CheckTriggerMemBreakpoint(DWORD addr,DWORD siz,int access)
{
	int i;

	if (!breakpoint_powered) return;
	i=DoesTriggerMemBreakpoint(addr,siz,access);
	if (i >= 0) SignalBreakPointTrigger(i);
}

void CheckTriggerIOBreakpoint(DWORD addr,DWORD siz,int access)
{
	int i;

	if (!breakpoint_powered) return;
	i=DoesTriggerIOBreakpoint(addr,siz,access);
	if (i >= 0) SignalBreakPointTrigger(i);
}

void CheckTriggerSWINTBreakpoint(int j)
{
	int i;

	if (!breakpoint_powered) return;
	i=DoesTriggerSWINTBreakpoint(j);
	if (i >= 0) SignalBreakPointTrigger(i);
}

void CheckTriggerHWINTBreakpoint(int j)
{
	int i;

	if (!breakpoint_powered) return;
	i=DoesTriggerHWINTBreakpoint(j);
	if (i >= 0) SignalBreakPointTrigger(i);
}

void SignalBreakPointTrigger(int idx)
{
	if (!breakpoint_powered) return;

	code_freerun=0;
	exec_show_breakpoint=1;
}

BOOL CALLBACK AddBreakDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	char buf[512];
	int i,u,from,too,t1,t2;
	char *ptr;

	if (msg == WM_INITDIALOG) {
		CheckDlgButton(hwnd,IDC_TYPE_MEMR,TRUE);
		CheckDlgButton(hwnd,IDC_TR_READ,TRUE);
		CheckDlgButton(hwnd,IDC_TR_WRITE,TRUE);
		return TRUE;
	}
	if (msg == WM_COMMAND) {
		if (wParam == IDOK) {
			u = IsDlgButtonChecked(hwnd,IDC_TR_READ) ? BPTR_READ : 0;
			u |= IsDlgButtonChecked(hwnd,IDC_TR_WRITE) ? BPTR_WRITE : 0;
			if (!u) {
				MessageBeep(0xFFFFFFFF);
				return FALSE;
			}

			i = -1;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_MEMR))		i = BPT_MEMRANGE;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_IOR))		i = BPT_IOPORTRANGE;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_SWR))		i = BPT_SWINTRANGE;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_HWR))		i = BPT_HWINTRANGE;

			if (i == -1) {
				MessageBeep(0xFFFFFFFF);
				return FALSE;
			}

			breakpoints[new_breakpoint_idx].type = i;
			breakpoints[new_breakpoint_idx].trigger = u;
			breakpoints[new_breakpoint_idx].enabled = 1;

			GetDlgItemText(hwnd,IDC_FROM,buf,500);
// convert 16:16 pointer if entered (seg:ofs)
			if (ptr = strstr(buf,":")) {
				*ptr=0; ptr++;
				t1 = strtoul(buf,NULL,0);
				t2 = strtoul(ptr,NULL,0);

				from = (t1<<4)+t2;
			}
// else assume it's a linear address
			else {
				from = strtoul(buf,NULL,0);
			}

			GetDlgItemText(hwnd,IDC_TO,buf,500);
// convert 16:16 pointer if entered (seg:ofs)
			if (ptr = strstr(buf,":")) {
				*ptr=0; ptr++;
				t1 = strtoul(buf,NULL,0);
				t2 = strtoul(ptr,NULL,0);

				too = (t1<<4)+t2;
			}
// else assume it's a linear address
			else {
				too = strtoul(buf,NULL,0);
			}

			if (i == BPT_MEMRANGE) {
				breakpoints[new_breakpoint_idx].mem_addr = from;
				breakpoints[new_breakpoint_idx].mem_addr_end = too;
			}
			if (i == BPT_IOPORTRANGE) {
				breakpoints[new_breakpoint_idx].io_addr = from;
				breakpoints[new_breakpoint_idx].io_addr_end = too;
			}
			if (i == BPT_SWINTRANGE) {
				breakpoints[new_breakpoint_idx].swint_addr = from;
				breakpoints[new_breakpoint_idx].swint_addr_end = too;
			}
			if (i == BPT_HWINTRANGE) {
				breakpoints[new_breakpoint_idx].hwint_addr = from;
				breakpoints[new_breakpoint_idx].hwint_addr_end = too;
			}

			EndDialog(hwnd,1);
		}
		if (wParam == IDCANCEL) {
			EndDialog(hwnd,0);
		}
	}

	return FALSE;
}

BOOL CALLBACK EditBreakDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	char buf[512];
	int i,u,from,too,t1,t2;
	char *ptr;

	if (msg == WM_INITDIALOG) {
		SetWindowText(hwnd,"Edit breakpoint");

		CheckDlgButton(hwnd,IDC_TR_READ,(breakpoints[new_breakpoint_idx].trigger & BPTR_READ)?1:0);
		CheckDlgButton(hwnd,IDC_TR_WRITE,(breakpoints[new_breakpoint_idx].trigger & BPTR_WRITE)?1:0);

		i = breakpoints[new_breakpoint_idx].type;
		CheckDlgButton(hwnd,IDC_TYPE_MEMR,i == BPT_MEMRANGE);
		CheckDlgButton(hwnd,IDC_TYPE_IOR,i == BPT_IOPORTRANGE);
		CheckDlgButton(hwnd,IDC_TYPE_SWR,i == BPT_SWINTRANGE);
		CheckDlgButton(hwnd,IDC_TYPE_HWR,i == BPT_HWINTRANGE);

		if (i == BPT_MEMRANGE) {
			from = breakpoints[new_breakpoint_idx].mem_addr;
			too = breakpoints[new_breakpoint_idx].mem_addr_end;
		}
		if (i == BPT_IOPORTRANGE) {
			from = breakpoints[new_breakpoint_idx].io_addr;
			too = breakpoints[new_breakpoint_idx].io_addr_end;
		}
		if (i == BPT_SWINTRANGE) {
			from = breakpoints[new_breakpoint_idx].swint_addr;
			too = breakpoints[new_breakpoint_idx].swint_addr_end;
		}
		if (i == BPT_HWINTRANGE) {
			from = breakpoints[new_breakpoint_idx].hwint_addr;
			too = breakpoints[new_breakpoint_idx].hwint_addr_end;
		}

		sprintf(buf,"0x%08X",from);
		SetDlgItemText(hwnd,IDC_FROM,buf);

		sprintf(buf,"0x%08X",too);
		SetDlgItemText(hwnd,IDC_TO,buf);

		return TRUE;
	}
	if (msg == WM_COMMAND) {
		if (wParam == IDOK) {
			u = IsDlgButtonChecked(hwnd,IDC_TR_READ) ? BPTR_READ : 0;
			u |= IsDlgButtonChecked(hwnd,IDC_TR_WRITE) ? BPTR_WRITE : 0;
			if (!u) {
				MessageBeep(0xFFFFFFFF);
				return FALSE;
			}

			i = -1;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_MEMR))		i = BPT_MEMRANGE;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_IOR))		i = BPT_IOPORTRANGE;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_SWR))		i = BPT_SWINTRANGE;
			if (IsDlgButtonChecked(hwnd,IDC_TYPE_HWR))		i = BPT_HWINTRANGE;

			if (i == -1) {
				MessageBeep(0xFFFFFFFF);
				return FALSE;
			}

			breakpoints[new_breakpoint_idx].type = i;
			breakpoints[new_breakpoint_idx].trigger = u;

			GetDlgItemText(hwnd,IDC_FROM,buf,500);
// convert 16:16 pointer if entered (seg:ofs)
			if (ptr = strstr(buf,":")) {
				*ptr=0; ptr++;
				t1 = strtoul(buf,NULL,0);
				t2 = strtoul(ptr,NULL,0);

				from = (t1<<4)+t2;
			}
// else assume it's a linear address
			else {
				from = strtoul(buf,NULL,0);
			}

			GetDlgItemText(hwnd,IDC_TO,buf,500);
// convert 16:16 pointer if entered (seg:ofs)
			if (ptr = strstr(buf,":")) {
				*ptr=0; ptr++;
				t1 = strtoul(buf,NULL,0);
				t2 = strtoul(ptr,NULL,0);

				too = (t1<<4)+t2;
			}
// else assume it's a linear address
			else {
				too = strtoul(buf,NULL,0);
			}

			if (i == BPT_MEMRANGE) {
				breakpoints[new_breakpoint_idx].mem_addr = from;
				breakpoints[new_breakpoint_idx].mem_addr_end = too;
			}
			if (i == BPT_IOPORTRANGE) {
				breakpoints[new_breakpoint_idx].io_addr = from;
				breakpoints[new_breakpoint_idx].io_addr_end = too;
			}
			if (i == BPT_SWINTRANGE) {
				breakpoints[new_breakpoint_idx].swint_addr = from;
				breakpoints[new_breakpoint_idx].swint_addr_end = too;
			}
			if (i == BPT_HWINTRANGE) {
				breakpoints[new_breakpoint_idx].hwint_addr = from;
				breakpoints[new_breakpoint_idx].hwint_addr_end = too;
			}

			EndDialog(hwnd,1);
		}
		if (wParam == IDCANCEL) {
			EndDialog(hwnd,0);
		}
	}

	return FALSE;
}

void RemoveBreakPoint(int x)
{
	breakpoints[x].type = 0;
	PackBreakPoints();
}

BOOL CALLBACK UserEditBreakDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	char buf[512];
	int x,i,j;

	if (msg == WM_INITDIALOG) {
		SetWindowText(hwnd,"Edit breakpoint");
		SendDlgItemMessage(hwnd,IDC_BREAKLIST,WM_SETFONT,(WPARAM)UserEditBreakDlgFont,0L);
		PackBreakPoints();
		SendMessage(hwnd,WM_USER+400,0,0L);
		return TRUE;
	}
	if (msg == WM_USER+400) {
// list breakpoints
		j = (int)SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_GETCURSEL,0,0L);
		SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_RESETCONTENT,0,0L);
		for (x=0;x < MAX_BREAKPOINTS && breakpoints[x].type > 0;x++) {
			sprintf(buf," %s %s",breakpoint_en[breakpoints[x].enabled?1:0],breakpoint_typestr[breakpoints[x].type]);
			i=strlen(buf);

			if (breakpoints[x].type == BPT_MEMRANGE)			sprintf(buf+i," @ 0x%08X-0x%08X",breakpoints[x].mem_addr,breakpoints[x].mem_addr_end);
			else if (breakpoints[x].type == BPT_IOPORTRANGE)	sprintf(buf+i," @ 0x%04X-0x%04X",breakpoints[x].io_addr,breakpoints[x].io_addr_end);
			else if (breakpoints[x].type == BPT_SWINTRANGE)		sprintf(buf+i," @ 0x%02X-0x%02X",breakpoints[x].swint_addr,breakpoints[x].swint_addr_end);
			else if (breakpoints[x].type == BPT_HWINTRANGE)		sprintf(buf+i," @ 0x%02X-0x%02X",breakpoints[x].hwint_addr,breakpoints[x].hwint_addr_end);

			if (breakpoints[x].trigger & BPTR_READ)		strcat(buf," R");
			else										strcat(buf,"  ");
			if (breakpoints[x].trigger & BPTR_WRITE)	strcat(buf,"W");
			else										strcat(buf," ");

			SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_ADDSTRING,0,(LPARAM)((DWORD)buf));
		}
		if (j >= x) j=x-1;
		SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_SETCURSEL,(WPARAM)j,0L);
	}
	if (msg == WM_COMMAND) {
		if (wParam == IDOK || wParam == IDCANCEL) {
			EndDialog(hwnd,0);
		}
		if (wParam == IDC_ADD) {
			x=i=0;
			while (x < MAX_BREAKPOINTS && breakpoints[x].type > 0) x++;

			if (x < MAX_BREAKPOINTS) {
				new_breakpoint_idx=x;

				if (DialogBox(hInst,MAKEINTRESOURCE(IDD_ADDBREAK),hwnd,(int (__stdcall *)())AddBreakDlgProc)) {
					SendMessage(hwnd,WM_USER+400,0,0L);
				}
				SetFocus(hwnd);
			}
		}
		if (wParam == IDC_REMOVE) {
			i = (int)SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_GETCURSEL,0,0L);
			if (i >= 0) {
				RemoveBreakPoint(i);
				SendMessage(hwnd,WM_USER+400,0,0L);
			}
		}
		if (wParam == IDC_ENABLE) {
			i = (int)SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_GETCURSEL,0,0L);
			if (i >= 0) {
				breakpoints[i].enabled = 1;
				SendMessage(hwnd,WM_USER+400,0,0L);
			}
		}
		if (wParam == IDC_DISABLE) {
			i = (int)SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_GETCURSEL,0,0L);
			if (i >= 0) {
				breakpoints[i].enabled = 0;
				SendMessage(hwnd,WM_USER+400,0,0L);
			}
		}
		if (wParam == IDC_EDIT) {
			i = (int)SendDlgItemMessage(hwnd,IDC_BREAKLIST,LB_GETCURSEL,0,0L);
			if (i >= 0) {
				new_breakpoint_idx=i;

				if (DialogBox(hInst,MAKEINTRESOURCE(IDD_ADDBREAK),hwnd,(int (__stdcall *)())EditBreakDlgProc)) {
					SendMessage(hwnd,WM_USER+400,0,0L);
				}
				SetFocus(hwnd);
			}
		}
	}

	return FALSE;
}

void InitBreakPointSys()
{
	int x,f;
	char buf[512];
	char brkf[64];

    for (x=0;x < MAX_BREAKPOINTS;x++) breakpoints[x].type = 0;
	breakpoint_powered=0;

	for (x=0,f=1;x < MAX_BREAKPOINTS && f;x++) {
		sprintf(brkf,"brk%d.type",x);
		breakpoints[x].type = GetPrivateProfileInt("breakpoints",brkf,0,common_ini_file);

		if (breakpoints[x].type > 0) {
			sprintf(brkf,"brk%d.trigger",x);
			breakpoints[x].trigger = GetPrivateProfileInt("breakpoints",brkf,0,common_ini_file);
			sprintf(brkf,"brk%d.enabled",x);
			breakpoints[x].enabled = GetPrivateProfileInt("breakpoints",brkf,0,common_ini_file);

			sprintf(brkf,"brk%d.mem_addr",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].mem_addr = strtol(buf,NULL,0);

			sprintf(brkf,"brk%d.mem_addr_end",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].mem_addr_end = strtol(buf,NULL,0);

			sprintf(brkf,"brk%d.io_addr",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].io_addr = strtol(buf,NULL,0);

			sprintf(brkf,"brk%d.io_addr_end",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].io_addr_end = strtol(buf,NULL,0);

			sprintf(brkf,"brk%d.swint_addr",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].swint_addr = strtol(buf,NULL,0);

			sprintf(brkf,"brk%d.swint_addr_end",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].swint_addr_end = strtol(buf,NULL,0);

			sprintf(brkf,"brk%d.hwint_addr",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].hwint_addr = strtol(buf,NULL,0);

			sprintf(brkf,"brk%d.hwint_addr_end",x);
			GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file);
			breakpoints[x].hwint_addr_end = strtol(buf,NULL,0);
		}
		else {
			f=0;
		}
	}
}

void CloseBreakPointSys()
{
	int x,f;
	char buf[512];
	char brkf[64];

	for (x=0,f=1;x < MAX_BREAKPOINTS && breakpoints[x].type > 0;x++) {
		sprintf(brkf,"brk%d.type",x); sprintf(buf,"%d",breakpoints[x].type);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.trigger",x); sprintf(buf,"%d",breakpoints[x].trigger);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.enabled",x); sprintf(buf,"%d",breakpoints[x].enabled);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.mem_addr",x); sprintf(buf,"0x%08X",breakpoints[x].mem_addr);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.mem_addr_end",x); sprintf(buf,"0x%08X",breakpoints[x].mem_addr_end);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.io_addr",x); sprintf(buf,"0x%08X",breakpoints[x].io_addr);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.io_addr_end",x); sprintf(buf,"0x%08X",breakpoints[x].io_addr_end);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.swint_addr",x); sprintf(buf,"0x%08X",breakpoints[x].swint_addr);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.swint_addr_end",x); sprintf(buf,"0x%08X",breakpoints[x].swint_addr_end);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.hwint_addr",x); sprintf(buf,"0x%08X",breakpoints[x].hwint_addr);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		sprintf(brkf,"brk%d.hwint_addr_end",x); sprintf(buf,"0x%08X",breakpoints[x].hwint_addr_end);
		WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
	}
	buf[0]=5;
	for (;x < MAX_BREAKPOINTS && buf[0];x++) {
		sprintf(brkf,"brk%d.type",x);
		if (!GetPrivateProfileString("breakpoints",brkf,"",buf,511,common_ini_file)) buf[0] = 0;

		if (buf[0]) {
			sprintf(buf,"");
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.trigger",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.enabled",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.mem_addr",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.mem_addr_end",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.io_addr",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.io_addr_end",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.swint_addr",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.swint_addr_end",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.hwint_addr",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
			sprintf(brkf,"brk%d.hwint_addr_end",x);
			WritePrivateProfileString("breakpoints",brkf,buf,common_ini_file);
		}
	}

	breakpoint_powered=0;
}

void PowerOnBreakPointSys()
{
	breakpoint_powered=1;
}

void PowerOffBreakPointSys()
{
	breakpoint_powered=0;
}

int IsBreakPointSysPowered()
{
	return breakpoint_powered;
}

void ResetBreakPointSys()
{
}

void PackBreakPoints()
{
	int x,x2;

	for (x=0;x < MAX_BREAKPOINTS && breakpoints[x].type > 0;x++);
	x2=x; x++;
	for (;x < MAX_BREAKPOINTS;x++) {
		if (breakpoints[x].type > 0) {
			memcpy(&breakpoints[x2],&breakpoints[x],sizeof(BREAKPOINT));
			x2++;
		}
	}
	while (x2 < MAX_BREAKPOINTS) {
		memset(&breakpoints[x2],0,sizeof(BREAKPOINT));
		x2++;
	}
}

void UserEditBreakPoints()
{
	if (!UserEditBreakDlgFont) {
		UserEditBreakDlgFont = CreateFont(-12,8,0,0,FW_NORMAL,
			FALSE,FALSE,FALSE,
			DEFAULT_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			FIXED_PITCH | FF_DONTCARE,
			"Terminal");
	}

	DialogBox(hInst,MAKEINTRESOURCE(IDD_BREAKPOINTS),hwndMain,(int (__stdcall *)())UserEditBreakDlgProc);
}
