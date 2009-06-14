
#include "global.h"
#include "resource.h"
#include <stdio.h>
#include "direct86.h"
#include "cputail.h"

// CPU trailing vars
CPULOC						prev_cpu_locations[CPULOCMAX];
int							prev_cpu_locations_in=0,prev_cpu_locations_out=0;
int							cpu_trails_powered=0;

void AddPrevCPUAddr(DWORD cs,DWORD ip)
{
	int i;

	i = (prev_cpu_locations_in + CPULOCMAX + -1)%CPULOCMAX;

	if (prev_cpu_locations[i].valid && prev_cpu_locations[i].cs == cs && prev_cpu_locations[i].ip == ip) {
		prev_cpu_locations_in = i;
		prev_cpu_locations[prev_cpu_locations_in].rep++;
	}
	else {
		prev_cpu_locations[prev_cpu_locations_in].valid=1;
		prev_cpu_locations[prev_cpu_locations_in].cs = cs;
		prev_cpu_locations[prev_cpu_locations_in].ip = ip;
		prev_cpu_locations[prev_cpu_locations_in].rep = 0;
	}

	prev_cpu_locations_in++;
	if (prev_cpu_locations_in >= CPULOCMAX) prev_cpu_locations_in=0;

	if (prev_cpu_locations_in == prev_cpu_locations_out) prev_cpu_locations_out++;
	if (prev_cpu_locations_out >= CPULOCMAX) prev_cpu_locations_out=0;
}

BOOL CALLBACK CPUTrailDlgProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	char buf[512];
	int i,j,c;

	if (msg == WM_INITDIALOG) {
		i = (prev_cpu_locations_in + CPULOCMAX + -1)%CPULOCMAX;
		j = (prev_cpu_locations_out + CPULOCMAX + -1)%CPULOCMAX;
		c = -1;
		while (i != j) {
			if (prev_cpu_locations[i].valid) {
				if (prev_cpu_locations[i].rep)	sprintf(buf,"%d: %04X:%08X (%d)",c,prev_cpu_locations[i].cs,prev_cpu_locations[i].ip,prev_cpu_locations[i].rep+1);
				else							sprintf(buf,"%d: %04X:%08X",c,prev_cpu_locations[i].cs,prev_cpu_locations[i].ip);
				SendDlgItemMessage(hwnd,IDC_TRAILLIST,LB_ADDSTRING,0,(LPARAM)((DWORD)buf));
			}

			i--;
			c -= prev_cpu_locations[i].rep+1;
			if (i < 0) i=CPULOCMAX-1;
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

void CPUTrailsView()
{
	if (!cpu_trails_powered) {
		MessageBox(hwndMain,"The CPU Trailing device is not powered or initialized in your virtual system configuration.\n\nBring up edit menu and select \"Add/Remove\" devices","Error",MB_OK);
		return;
	}

	DialogBox(hInst,MAKEINTRESOURCE(IDD_CPUTRAIL),hwndMain,CPUTrailDlgProc);
}

void CPUTrailsInit()
{
	cpu_trails_powered=0;
	prev_cpu_locations_in=prev_cpu_locations_out=0;
}

void CPUTrailsClose()
{
	cpu_trails_powered=0;
	prev_cpu_locations_in=prev_cpu_locations_out=0;
}

void CPUTrailsPowerOn()
{
	cpu_trails_powered=1;
}

void CPUTrailsPowerOff()
{
	cpu_trails_powered=0;
	prev_cpu_locations_in=prev_cpu_locations_out=0;
}

void CPUTrailsReset()
{
	prev_cpu_locations_in=prev_cpu_locations_out=0;
}

int IsCPUTrailsPowered()
{
	return cpu_trails_powered;
}
