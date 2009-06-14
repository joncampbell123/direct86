
#include "global.h"
#include "resource.h"
#include "direct86.h"
#include "cpustkw.h"
#include <stdio.h>

STACKWOP				stack_watch_ops[MAXSTACKWOPS];
int						stack_watch_ops_r,stack_watch_ops_w;
int						stack_watch_ops_r_dsp=0;
int						stack_watch_ops_w_dsp=0;
int						cpu_stack_power;
FILE*					stack_log;

int GetEntrySPDelta(int i,int dsp)
{
	if (stack_watch_ops[i].type == SWT_CALLNEAR)			dsp -= 2;
	else if (stack_watch_ops[i].type == SWT_CALLFAR)		dsp -= 4;
	else if (stack_watch_ops[i].type == SWT_CALLINT)		dsp -= 6;
	else if (stack_watch_ops[i].type == SWT_DISREGARD)		dsp = 0;
	else if (stack_watch_ops[i].type == SWT_RET)			dsp += 2;
	else if (stack_watch_ops[i].type == SWT_RETF)			dsp += 4;
	else if (stack_watch_ops[i].type == SWT_IRET)			dsp += 6;
	else if (stack_watch_ops[i].type == SWT_PUSH)			dsp -= stack_watch_ops[i].old_sp - stack_watch_ops[i].new_sp;
	else if (stack_watch_ops[i].type == SWT_POP)			dsp += stack_watch_ops[i].new_sp - stack_watch_ops[i].old_sp;
	else if (stack_watch_ops[i].type == SWT_PUSHALLOC)		dsp -= stack_watch_ops[i].old_sp - stack_watch_ops[i].new_sp;
	else if (stack_watch_ops[i].type == SWT_POPIGNORE)		dsp += stack_watch_ops[i].new_sp - stack_watch_ops[i].old_sp;
	else if (stack_watch_ops[i].type == SWT_HWCALLINT)		dsp -= 6;
	else													dsp = 0;

	return dsp;
}

int GenStringFromEntry(int i,char *buf,int dsp)
{
	char *bufr;

	memset(buf,0,32);
	dsp = GetEntrySPDelta(i,dsp);
	buf[0] = (dsp < 0) ? '-' : '+';
	sprintf(buf+1,"%-5d < ",abs(dsp));

	bufr = buf+strlen(buf);

	if (stack_watch_ops[i].type == SWT_CALLNEAR)
		sprintf(bufr,"%04X:%04X near   call to this addr   from %04X:%04X",stack_watch_ops[i].cs,stack_watch_ops[i].ip,stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip);
	else if (stack_watch_ops[i].type == SWT_CALLFAR)
		sprintf(bufr,"%04X:%04X far    call to this addr   from %04X:%04X",stack_watch_ops[i].cs,stack_watch_ops[i].ip,stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip);
	else if (stack_watch_ops[i].type == SWT_CALLINT)
		sprintf(bufr,"%04X:%04X int %02X call to this addr   from %04X:%04X",stack_watch_ops[i].cs,stack_watch_ops[i].ip,stack_watch_ops[i].intnum,stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip);
	else if (stack_watch_ops[i].type == SWT_DISREGARD)
		sprintf(bufr,"%04X:%04X stack disregard (SS:SP %04X:%04X => %04X:%04X)",stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip,stack_watch_ops[i].old_ss,stack_watch_ops[i].old_sp,stack_watch_ops[i].new_ss,stack_watch_ops[i].new_sp);
	else if (stack_watch_ops[i].type == SWT_RET)
		sprintf(bufr,"%04X:%04X near   return to this addr from %04X:%04X",stack_watch_ops[i].cs,stack_watch_ops[i].ip,stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip);
	else if (stack_watch_ops[i].type == SWT_RETF)
		sprintf(bufr,"%04X:%04X far    return to this addr from %04X:%04X",stack_watch_ops[i].cs,stack_watch_ops[i].ip,stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip);
	else if (stack_watch_ops[i].type == SWT_IRET)
		sprintf(bufr,"%04X:%04X int    return to this addr from %04X:%04X",stack_watch_ops[i].cs,stack_watch_ops[i].ip,stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip);
	else if (stack_watch_ops[i].type == SWT_PUSH)
		sprintf(bufr,"%04X:%04X push   %d bytes on stack",stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip,stack_watch_ops[i].old_sp - stack_watch_ops[i].new_sp);
	else if (stack_watch_ops[i].type == SWT_POP)
		sprintf(bufr,"%04X:%04X pop    %d bytes on stack",stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip,stack_watch_ops[i].new_sp - stack_watch_ops[i].old_sp);
	else if (stack_watch_ops[i].type == SWT_PUSHALLOC)
		sprintf(bufr,"%04X:%04X alloc  %d bytes on stack",stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip,stack_watch_ops[i].old_sp - stack_watch_ops[i].new_sp);
	else if (stack_watch_ops[i].type == SWT_POPIGNORE)
		sprintf(bufr,"%04X:%04X free   %d bytes on stack",stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip,stack_watch_ops[i].new_sp - stack_watch_ops[i].old_sp);
	else if (stack_watch_ops[i].type == SWT_HWCALLINT)
		sprintf(bufr,"%04X:%04X H/W int %02X call to this addr from %04X:%04X",stack_watch_ops[i].cs,stack_watch_ops[i].ip,stack_watch_ops[i].intnum,stack_watch_ops[i].from_cs,stack_watch_ops[i].from_ip);
	else
		sprintf(bufr,"%04X:%04X [unknown]",stack_watch_ops[i].cs,stack_watch_ops[i].ip);

	return dsp;
}

void ClearStackWatch()
{
	int x;

	for (x=0;x < MAXSTACKWOPS;x++) stack_watch_ops[x].type=0;
	stack_watch_ops_r=stack_watch_ops_w=0;
	stack_watch_ops_r_dsp=0;
	stack_watch_ops_w_dsp=0;
}

void AddStackWatchEntryINT(int type,DWORD cs,DWORD ip,DWORD ocs,DWORD oip,DWORD oss,DWORD nss,DWORD osp,DWORD nsp,int num)
{
	if (!cpu_stack_power) return;

	stack_watch_ops[stack_watch_ops_w].intnum = num;
	stack_watch_ops[stack_watch_ops_w].new_sp = nsp;
	stack_watch_ops[stack_watch_ops_w].new_ss = nss;
	stack_watch_ops[stack_watch_ops_w].old_sp = osp;
	stack_watch_ops[stack_watch_ops_w].old_ss = oss;
	stack_watch_ops[stack_watch_ops_w].cs = cs;
	stack_watch_ops[stack_watch_ops_w].ip = ip;
	stack_watch_ops[stack_watch_ops_w].from_cs = ocs;
	stack_watch_ops[stack_watch_ops_w].from_ip = oip;
	stack_watch_ops[stack_watch_ops_w].type = type;

//	if (!stack_log) stack_log = fopen("c:\\jmc\\stackl.txt","wb");
//	if (stack_log) {
//		stack_watch_ops_w_dsp = GenStringFromEntry(stack_watch_ops_w,buf,stack_watch_ops_w_dsp);
//		fprintf(stack_log,"%s\n",buf);
//	}

	stack_watch_ops_w++;
	if (stack_watch_ops_w >= MAXSTACKWOPS)			stack_watch_ops_w=0;
	if (stack_watch_ops_r == stack_watch_ops_w) {
		stack_watch_ops_r_dsp = GetEntrySPDelta(stack_watch_ops_r,stack_watch_ops_r_dsp);
		stack_watch_ops_r++;
	}
}

void AddStackWatchEntry(int type,DWORD cs,DWORD ip,DWORD ocs,DWORD oip,DWORD oss,DWORD nss,DWORD osp,DWORD nsp)
{
	AddStackWatchEntryINT(type,cs,ip,ocs,oip,oss,nss,osp,nsp,0);
}

BOOL CALLBACK CPUStackWatchDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	char buf[512];
	int i,j,dsp;

	if (msg == WM_INITDIALOG) {
		i = stack_watch_ops_r;
		j = stack_watch_ops_w;
		dsp = stack_watch_ops_r_dsp;
		while (i != j) {
			if (stack_watch_ops[i].type) {
				dsp=GenStringFromEntry(i,buf,dsp);
				SendDlgItemMessage(hwnd,IDC_LIST,LB_ADDSTRING,0,(LPARAM)((DWORD)buf));
			}

			i++; if (i >= MAXSTACKWOPS) i=0;
		}

		return TRUE;
	}
	if (msg == WM_COMMAND) {
		if (wParam == IDOK || wParam == IDCANCEL) {
			EndDialog(hwnd,0);
		}
	}

	return FALSE;
}

void StackWatchView()
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_CPUSTACKWATCH),hwndMain,CPUStackWatchDlgProc);
}

void CPUStackWatchInit()
{
	ClearStackWatch();
	cpu_stack_power=0;
}

void CPUStackWatchClose()
{
}

void CPUStackWatchPowerOn()
{
	cpu_stack_power=1;
}

void CPUStackWatchPowerOff()
{
	cpu_stack_power=0;
}

void CPUStackWatchReset()
{
	ClearStackWatch();
}

int IsCPUStackWatchPowered()
{
	return cpu_stack_power;
}
