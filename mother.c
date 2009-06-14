
#include "global.h"
#include "resource.h"
#include <stdio.h>
#include "direct86.h"
#include "fdsim.h"
#include "hdsim.h"
#include "bios.h"
#include "mother.h"
#include "cpuexec.h"
#include "cpudec.h"
#include "cmos.h"
#include "keyboard.h"
#include "addrbus.h"
#include "hardware.h"
#include "ram.h"
#include "cmos.h"
#include "timer.h"
#include "stockvga.h"
#include "pic.h"
#include "brkpts.h"
#include "cpustkw.h"
#include "cputail.h"
#include "direct86.h"
#include "lib.h"

EMUDEVICE default_config_dev[] = {
	{1,"Device I/O bus"						,InitHardware		,CloseHardware		,PowerOnHardware		,PowerOffHardware		,ResetHardware		,CycleHardware				,IsHardwarePowered},
	{1,"Address bus"						,AddressBusEmuInit	,AddressBusEmuClose	,AddressBusEmuPowerOn	,AddressBusEmuPowerOff	,AddressBusReset	,NULL						,IsAddressBusPowered},
	{1,"Random Access Memory"				,RAMInit			,RAMClose			,RAMPowerOn				,RAMPowerOff			,RAMReset			,NULL						,IsRAMPowered},
	{1,"Central Processing Unit"			,InitExecutioneer	,CloseExecutioneer	,ExecutioneerPowerOn	,ExecutioneerPowerOff	,ExecutioneerReset	,NULL						,ExecutioneerIsPowered},
	{1,"ROM BIOS"							,InitBIOS			,CloseBIOS			,PowerOnBIOS			,PowerDownBIOS			,ResetBIOS			,NULL						,IsBIOSPowered},
	{1,"VGA Display"						,VGAInit			,VGAClose			,VGAPowerOn				,VGAPowerOff			,VGAReset			,NULL						,VGAIsPowered},
	{1,"System Timer"						,InitTimer			,CloseTimer			,TimerPowerUp			,TimerPowerDown			,TimerReset			,TimerCycle					,TimerIsPowered},
	{1,"CMOS and Real Time Clock"			,CMOSInit			,CMOSClose			,CMOSPowerUp			,CMOSPowerDown			,CMOSReset			,CMOSCycle					,CMOSIsPowered},
	{1,"System Keyboard"					,KeyboardInit		,KeyboardClose		,KeyboardPowerOn		,KeyboardPowerOff		,KeyboardReset		,KeyboardControllerCycle	,KeyboardIsPowered},
	{1,"Programmable Interrupt Controller"	,InitPIC			,ClosePIC			,PICPowerOn				,PICPowerOff			,PICReset			,PICCycle					,PICIsPowered},
};

EMUDEVICE total_dev[] = {
	{1,"Device I/O bus"						,InitHardware		,CloseHardware		,PowerOnHardware		,PowerOffHardware		,ResetHardware		,CycleHardware				,IsHardwarePowered},
	{1,"Address bus"						,AddressBusEmuInit	,AddressBusEmuClose	,AddressBusEmuPowerOn	,AddressBusEmuPowerOff	,AddressBusReset	,NULL						,IsAddressBusPowered},
	{1,"Random Access Memory"				,RAMInit			,RAMClose			,RAMPowerOn				,RAMPowerOff			,RAMReset			,NULL						,IsRAMPowered},
	{1,"Central Processing Unit"			,InitExecutioneer	,CloseExecutioneer	,ExecutioneerPowerOn	,ExecutioneerPowerOff	,ExecutioneerReset	,NULL						,ExecutioneerIsPowered},
	{1,"CPU Breakpoint Debugging device"	,InitBreakPointSys	,CloseBreakPointSys	,PowerOnBreakPointSys	,PowerOffBreakPointSys	,ResetBreakPointSys	,NULL						,IsBreakPointSysPowered},
	{1,"CPU Execution trailing device"		,CPUTrailsInit		,CPUTrailsClose		,CPUTrailsPowerOn		,CPUTrailsPowerOff		,CPUTrailsReset		,NULL						,IsCPUTrailsPowered},
	{1,"CPU Stack monitor device"			,CPUStackWatchInit	,CPUStackWatchClose	,CPUStackWatchPowerOn   ,CPUStackWatchPowerOff  ,CPUStackWatchReset ,NULL						,IsCPUStackWatchPowered},
	{1,"ROM BIOS"							,InitBIOS			,CloseBIOS			,PowerOnBIOS			,PowerDownBIOS			,ResetBIOS			,NULL						,IsBIOSPowered},
	{1,"VGA Display"						,VGAInit			,VGAClose			,VGAPowerOn				,VGAPowerOff			,VGAReset			,NULL						,VGAIsPowered},
	{1,"System Timer"						,InitTimer			,CloseTimer			,TimerPowerUp			,TimerPowerDown			,TimerReset			,TimerCycle					,TimerIsPowered},
	{1,"CMOS and Real Time Clock"			,CMOSInit			,CMOSClose			,CMOSPowerUp			,CMOSPowerDown			,CMOSReset			,CMOSCycle					,CMOSIsPowered},
	{1,"System Keyboard"					,KeyboardInit		,KeyboardClose		,KeyboardPowerOn		,KeyboardPowerOff		,KeyboardReset		,KeyboardControllerCycle	,KeyboardIsPowered},
	{1,"Programmable Interrupt Controller"	,InitPIC			,ClosePIC			,PICPowerOn				,PICPowerOff			,PICReset			,PICCycle					,PICIsPowered},
};
#define MAX_ADD_DEVICES (sizeof(total_dev) / sizeof(total_dev[0]))

#define MAX_DEFAULT_DEVICES (sizeof(default_config_dev) / sizeof(default_config_dev[0]))

EMUDEVICE					sys_devices[MAX_DEVICES];
int							sys_devices_next=0;
BOOL						PowerOn=FALSE;	// used to indicate power on/off state
BOOL						CPUpower=FALSE;	// CPUpower = whether or not the CPU has enough power to run
int							CPUpower_max=0;	// CPUpower_lev = "charge" of power for the CPU
int							CPUpower_min=0;	// CPUpower_max = maximum "charge" of power for the CPU to run off of
int							CPUpower_lev=0;	// CPUpower_min = minimum "charge" of power that the CPU can function with

void ComputerReset()
{
	int x;
	char buf[512];

	if (!PowerOn) ComputerPowerOn();

	WriteStatus("Initializing memory and CPU regs");

// initialize fake computer
	debug_window_offset32 = 0xFFFF0;
	debug_window_offset_cs = 0xF000;
	debug_window_offset_ip = 0xFFF0;

// initialize other fake hardware parts
	for (x=0;x < MAX_DEVICES;x++) {
		if (sys_devices[x].plugged_in) {
			sprintf(buf,"Resetting %s",sys_devices[x].device_name);
			WriteStatus(buf);
			if (sys_devices[x].resetdev) sys_devices[x].resetdev();
		}
	}

	RepaintSourceDump(FALSE);
	WriteStatus(NULL);
}

int FindDevByName(EMUDEVICE *list,int listitems,char *name)
{
	int i,d;

	d=0;
	for (i=0;i < listitems && !d;) {
		if (!list[i].device_name)						i++;
		else if (!strcmpi(list[i].device_name,name))	d=1;
		else											i++;
	}

	if (!d) return -1;
	return i;
}

void ComputerInit()
{
	int x,i,idx,installed;
	char buf[512];
	char devn[64];

	for (x=0;x < MAX_DEVICES;x++) sys_devices[x].plugged_in=0;
	sys_devices_next=0;

// load configuration from INI file
	i=0;
	idx=0;
	installed=0;
	x=1;
	while (x) {
		sprintf(devn,"dev%d",idx); idx++;
		GetPrivateProfileString("motherboard",devn,"",buf,511,common_ini_file);

// nothing? then stop
		if (!buf[0]) {
			x=0;
		}
		else {
// check if it's already there. if not, install
			i=FindDevByName(sys_devices,MAX_DEVICES,buf);
			if (i == -1) {
				i=FindDevByName(total_dev,MAX_ADD_DEVICES,buf);
				if (i == -1)			MessageBox(hwndMain,buf,"Unknown device",MB_OK);
				else					{ ComputerAddDevice(&total_dev[i]); installed++; }
			}
			else {
				MessageBox(hwndMain,buf,"device already installed",MB_OK);
			}
		}
	}

// if none installed, set up default configuration
	if (!installed) {
// scan initialization list and add "default" devices
		for (x=0;x < MAX_DEFAULT_DEVICES;x++) ComputerAddDevice(&default_config_dev[x]);
	}
}

void ComputerClose()
{
	int x,ix;
	char devn[64];

	for (x=0,ix=0;x < MAX_DEVICES;x++) {
// make note for next time we're started up
		if (sys_devices[x].plugged_in) {
			sprintf(devn,"dev%d",ix);
			WritePrivateProfileString("motherboard",devn,sys_devices[x].device_name,common_ini_file);
			ix++;
		}

		if (sys_devices[x].plugged_in && sys_devices[x].closedev) {
			sys_devices[x].closedev();
		}
		sys_devices[x].plugged_in=0;
	}
}

void ComputerPowerOn()
{
	int x;

	for (x=0;x < MAX_DEVICES;x++) {
		if (sys_devices[x].plugged_in && sys_devices[x].powerondev) {
			sys_devices[x].powerondev();
		}
	}
	PowerOn=1;
}

void ComputerPowerOff()
{
	int x;

	for (x=0;x < MAX_DEVICES;x++) {
		if (sys_devices[x].plugged_in && sys_devices[x].poweroffdev) {
			sys_devices[x].poweroffdev();
		}
	}
	PowerOn=0;
}

void ComputerCycle()
{
	int x;

	if (!PowerOn) return;

	for (x=0;x < MAX_DEVICES;x++) {
		if (sys_devices[x].plugged_in && sys_devices[x].buscycledev) {
			sys_devices[x].buscycledev();
		}
	}
}

int ComputerAddDevice(EMUDEVICE *dev)
{
	int idx;

	if (sys_devices_next >= MAX_DEVICES) return -1;

	idx = sys_devices_next;
	memcpy(&sys_devices[sys_devices_next],dev,sizeof(EMUDEVICE));
	while (sys_devices[sys_devices_next].plugged_in && sys_devices_next < MAX_DEVICES) sys_devices_next++;

	if (sys_devices_next >= MAX_DEVICES) {
		sys_devices_next = 0;
		while (sys_devices[sys_devices_next].plugged_in && sys_devices_next < MAX_DEVICES) sys_devices_next++;
	}

	if (sys_devices[idx].plugged_in && sys_devices[idx].initdev) sys_devices[idx].initdev();
	if (sys_devices[idx].plugged_in && sys_devices[idx].powerondev && PowerOn) sys_devices[idx].powerondev();

	return idx;
}

int ComputerRemoveDevice(int idx)
{
	if (idx < 0 || idx >= MAX_DEVICES) return 0;

	if (sys_devices[idx].plugged_in && sys_devices[idx].closedev) sys_devices[idx].closedev();

	if (idx < (MAX_DEVICES-1)) memmove(&sys_devices[idx],&sys_devices[idx+1],sizeof(EMUDEVICE)*(MAX_DEVICES - (idx+1)));
	memset(&sys_devices[MAX_DEVICES-1],0,sizeof(EMUDEVICE));
	while (sys_devices[idx].plugged_in) idx++;
	sys_devices_next = idx;

	return 1;
}

int ComputerResetDevice(int idx)
{
	if (!PowerOn) return 0;
	if (idx < 0 || idx >= MAX_DEVICES) return 0;
	if (!sys_devices[idx].plugged_in || !sys_devices[idx].resetdev) return 0;

	sys_devices[idx].resetdev();

	return 1;
}

int ComputerPowerOffDevice(int idx)
{
	if (!PowerOn) return 0;
	if (idx < 0 || idx >= MAX_DEVICES) return 0;
	if (!sys_devices[idx].plugged_in && !sys_devices[idx].poweroffdev) return 0;

	sys_devices[idx].poweroffdev();

	return 1;
}

int ComputerPowerOnDevice(int idx)
{
	if (!PowerOn) return 0;
	if (idx < 0 || idx >= MAX_DEVICES) return 0;
	if (!sys_devices[idx].plugged_in && !sys_devices[idx].powerondev) return 0;

	sys_devices[idx].powerondev();

	return 1;
}

int ComputerIsDevicePowered(int idx)
{
	int r;

	if (!PowerOn)						return 0;
	if (idx < 0 || idx >= MAX_DEVICES)	return 0;
	if (!sys_devices[idx].plugged_in)	return 0;

	if (sys_devices[idx].is_powered)		r=sys_devices[idx].is_powered();
	else									r=1;

	return r;
}

int dlgadd_map[MAX_ADD_DEVICES];
BOOL CALLBACK DlgAddDevConf(HWND hwnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	int wID,x,i,j,o;

	if (iMessage == WM_INITDIALOG) {
		o=0;
		for (i=0;i < MAX_ADD_DEVICES;i++) {
			x = -1;
			for (j=0;j < MAX_DEVICES && x < 0;j++) {
				if (sys_devices[j].plugged_in) {
					if (!memcmp(&sys_devices[j],&total_dev[i],sizeof(EMUDEVICE))) x = j;
				}
			}

			if (x < 0) {
				dlgadd_map[o++] = i;
				SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_ADDSTRING,0,(LPARAM)((DWORD)total_dev[i].device_name));
			}
		}
		while (o < MAX_ADD_DEVICES) dlgadd_map[o++] = -1;
		SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_SETCURSEL,0,0L);

		return TRUE;
	}
	else if (iMessage == WM_COMMAND) {
		wID = (int)wParam;
		if (wID == IDCANCEL) EndDialog(hwnd,0);
		if (wID == IDOK) {
			x = SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_GETCURSEL,0,0L);
			if (x >= 0) {
				x = dlgadd_map[x];
				if (x >= 0) {
					ComputerAddDevice(&total_dev[x]);
					EndDialog(hwnd,1);
				}
			}
		}
	}
	return FALSE;
}

int dlg_map[MAX_DEVICES];
BOOL CALLBACK DlgDevConf(HWND hwnd,UINT iMessage,WPARAM wParam,LPARAM lParam)
{
	int wID,x,i,j;
	char buf[800];

	if (iMessage == WM_INITDIALOG) {
		DlgDevConf(hwnd,WM_USER+600,0,0L);
		return TRUE;
	}
	else if (iMessage == WM_USER+600) {
		i = SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_GETCURSEL,0,0L);
		SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_RESETCONTENT,0,0L);
		j=0;
		for (x=0;x < MAX_DEVICES;x++) {
			if (sys_devices[x].plugged_in) {
				sprintf(buf,"%s",sys_devices[x].device_name);
				if (!ComputerIsDevicePowered(x)) strcat(buf," (unpowered)");

				SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_ADDSTRING,0,(LPARAM)((DWORD)buf));
				dlg_map[j++] = x;
			}
		}
		SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_SETCURSEL,i,0L);
		while (j < MAX_DEVICES) dlg_map[j++] = -1;
	}
	else if (iMessage == WM_COMMAND) {
		wID = (int)wParam;
		if (wID == IDCANCEL) EndDialog(hwnd,0);
		if (wID == IDC_POWEROFF) {
			x = SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_GETCURSEL,0,0L);
			if (x >= 0) {
				ComputerPowerOffDevice(dlg_map[x]);
				DlgDevConf(hwnd,WM_USER+600,0,0L);
			}
		}
		if (wID == IDC_POWERON) {
			x = SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_GETCURSEL,0,0L);
			if (x >= 0) {
				ComputerPowerOnDevice(dlg_map[x]);
				DlgDevConf(hwnd,WM_USER+600,0,0L);
			}
		}
		if (wID == IDC_RESET) {
			x = SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_GETCURSEL,0,0L);
			if (x >= 0) {
				ComputerResetDevice(dlg_map[x]);
				DlgDevConf(hwnd,WM_USER+600,0,0L);
			}
		}
		if (wID == IDC_REMOVE) {
			x = SendDlgItemMessage(hwnd,IDC_DEVLIST,LB_GETCURSEL,0,0L);
			if (x >= 0) {
				ComputerRemoveDevice(dlg_map[x]);
				DlgDevConf(hwnd,WM_USER+600,0,0L);
			}
		}
		if (wID == IDC_ADD) {
#ifdef WIN95
			DialogBox(hInst,MAKEINTRESOURCE(IDD_ADDDEVICES),hwnd,(int (__stdcall *)(struct HWND__ *,unsigned int,unsigned int,long))DlgAddDevConf);
#else
			DialogBox(hInst,MAKEINTRESOURCE(IDD_ADDDEVICES),hwnd,(int (__stdcall *)())DlgAddDevConf);
#endif
			SetFocus(hwnd);
			DlgDevConf(hwnd,WM_USER+600,0,0L);
		}
	}
	return FALSE;
}

void ComputerDevicesConfig()
{
#ifdef WIN95
	DialogBox(hInst,MAKEINTRESOURCE(IDD_DEVICES),hwndMain,(int (__stdcall *)(struct HWND__ *,unsigned int,unsigned int,long))DlgDevConf);
#else
	DialogBox(hInst,MAKEINTRESOURCE(IDD_DEVICES),hwndMain,(int (__stdcall *)())DlgDevConf);
#endif
	SetFocus(hwndMain);
}
