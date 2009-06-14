
// RAM "Random Access Memory" emulation

#include "global.h"
#include <stdio.h>
#include "resource.h"
#include "direct86.h"
#include "ram.h"
#include "addrbus.h"

DWORD				addrbus_access_ram(int size,int mod,DWORD dat,DWORD addr);
DWORD				addrbus_access_exram(int size,int mod,DWORD dat,DWORD addr);

int					ram_power=0;
BYTE*				conventional_ram=NULL;
int					conventional_ram_size=0;
BYTE*				extended_ram=NULL;
int					extended_ram_size=0;

void RAMInit()
{
	if (conventional_ram) return;

	conventional_ram_size = GetPrivateProfileInt("RAM","ConvSize",640,inifile)*1024;
	conventional_ram_size = (conventional_ram_size+4095) & ~4095;

	extended_ram_size = GetPrivateProfileInt("RAM","ExtSize",2048,inifile)*1024;
	extended_ram_size = (extended_ram_size+4095) & ~4095;

	if (conventional_ram_size > 0) {
		conventional_ram = malloc(conventional_ram_size);
		if (!conventional_ram) {
			conventional_ram_size=0;
			MessageBox(hwndMain,"ERR: Unable to allocate memory to emulate conventional RAM!","",MB_OK);
		}
	}
	else {
		conventional_ram = NULL;
	}

	if (extended_ram_size > 0) {
		extended_ram = malloc(extended_ram_size);
		if (!extended_ram) {
			extended_ram_size=0;
			MessageBox(hwndMain,"ERR: Unable to allocate memory to emulate extended RAM!","",MB_OK);
		}
	}
	else {
		extended_ram = NULL;
	}

	ram_power=0;
	RAMReset();
}

void RAMClose()
{
	if (conventional_ram) free(conventional_ram);
	if (extended_ram) free(extended_ram);

	conventional_ram=NULL;
	extended_ram=NULL;
}

void RAMReset()
{
	int x;

	if (conventional_ram)	memset(conventional_ram,0,conventional_ram_size);
	if (extended_ram)		memset(extended_ram,0,extended_ram_size);

	if (ram_power) {
		if (conventional_ram)	for (x=0;x < conventional_ram_size;x += 0x1000) addrbus_make_page_adaptor((DWORD)x,addrbus_access_ram);
		if (extended_ram)		for (x=0;x < extended_ram_size;x += 0x1000) addrbus_make_page_adaptor((DWORD)(x+0x100000),addrbus_access_exram);
	}
	else {
		if (conventional_ram)	for (x=0;x < conventional_ram_size;x += 0x1000) addrbus_make_page_empty((DWORD)x);
		if (extended_ram)		for (x=0;x < extended_ram_size;x += 0x1000) addrbus_make_page_empty((DWORD)(x+0x100000));
	}
}

int IsRAMPowered()
{
	return ram_power;
}

void RAMPowerOn()
{
	ram_power=1;
	RAMReset();
}

void RAMPowerOff()
{
	ram_power=0;
	RAMReset();
	RAMClose();
	RAMInit();
}

DWORD addrbus_access_ram(int size,int mod,DWORD dat,DWORD addr)
{
	DWORD r;
	int o,x;
	BYTE *ptr;

	if (!ram_power || !conventional_ram) return 0xFFFFFFFF;

	r=0;

	if (size > 4) size=4;
	if (!size) return 0;

	if (mod == AB_MOD_READ || mod == AB_MOD_SNAPSHOT) {
		addr += size-1;
		x = 0;
		o = addr - conventional_ram_size;
		ptr = conventional_ram+addr;
		while (o > 0) {
			r = (r<<8) | 0xFF;
			ptr--;
			o--;
			x++;
		}

		for (;x < size;x++) r = (r<<8) | ((DWORD)(*ptr--));
	}

	if (mod == AB_MOD_WRITE || mod == AB_MOD_SNAPSHOTWRITE) {
		o = addr - conventional_ram_size;
		if (o > 0) size -= o;

		ptr = conventional_ram+addr;
		for (x=0;x < size;x++) {
			*ptr++ = (BYTE)(dat&0xFF);
			dat >>= 8;
		}
	}

	return r;
}

DWORD addrbus_access_exram(int size,int mod,DWORD dat,DWORD addr)
{
	DWORD r;
	int o,x;
	BYTE *ptr;

	if (!ram_power || !extended_ram) return 0xFFFFFFFF;

	r=0;

	if (size > 4) size=4;
	if (!size) return 0;
	if (addr < 0x100000) return 0xFFFFFFFF;		// why are we responding if addr < 1M boundary?!?!?
	addr -= 0x100000;

	if (mod == AB_MOD_READ || mod == AB_MOD_SNAPSHOT) {
		addr += size-1;
		x = 0;
		o = addr - extended_ram_size;
		ptr = extended_ram+addr;
		while (o > 0) {
			r = (r<<8) | 0xFF;
			ptr--;
			o--;
			x++;
		}

		for (;x < size;x++) r = (r<<8) | ((DWORD)(*ptr--));
	}

	if (mod == AB_MOD_WRITE || mod == AB_MOD_SNAPSHOTWRITE) {
		o = addr - extended_ram_size;
		if (o > 0) size -= o;

		ptr = extended_ram+addr;
		for (x=0;x < size;x++) {
			*ptr++ = (BYTE)(dat&0xFF);
			dat >>= 8;
		}
	}

	return r;
}

static char *convmem_sizes[] = {
	"None",
	"4KB",
	"8KB",
	"16KB",
	"24KB",
	"32KB",
	"48KB",
	"64KB",
	"96KB",
	"128KB",
	"192KB",
	"256KB",
	"320KB",
	"384KB",
	"448KB",
	"512KB",
	"576KB",
	"640KB",
	NULL};

static char *extmem_sizes[] = {
	"None",
	"64KB",
	"128KB",
	"256KB",
	"512KB",
	"1024KB",
	"1536KB",
	"2048KB",
	"3072KB",
	"4096KB",
	"8092KB",
	"16384KB",
	"24476KB",
	"32768KB",
	NULL};

BOOL CALLBACK RamConfigDlg(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	int x,sz,cvm,exm;
	char buf[64];

	if (msg == WM_INITDIALOG) {
		cvm = GetPrivateProfileInt("RAM","ConvSize",640,inifile);
		cvm = (cvm + 3) & ~3;

		exm = GetPrivateProfileInt("RAM","ExtSize",2048,inifile);
		exm = (exm + 3) & ~3;

		for (x=0;convmem_sizes[x];x++) SendMessage(GetDlgItem(hwnd,IDC_RAM_CONVAMOUNT),CB_ADDSTRING,0,(LPARAM)convmem_sizes[x]);

		sz = atoi(convmem_sizes[0]);
		for (x=0;convmem_sizes[x] && sz < cvm;) {
			x++;
			sz = atoi(convmem_sizes[x]);
		}
		if (!convmem_sizes[x]) x--;
		SendMessage(GetDlgItem(hwnd,IDC_RAM_CONVAMOUNT),CB_SETCURSEL,(WPARAM)x,0L);

		for (x=0;extmem_sizes[x];x++) SendMessage(GetDlgItem(hwnd,IDC_RAM_EXTAMOUNT),CB_ADDSTRING,0,(LPARAM)extmem_sizes[x]);

		sz = atoi(extmem_sizes[0]);
		for (x=0;extmem_sizes[x] && sz < exm;) {
			x++;
			sz = atoi(extmem_sizes[x]);
		}
		if (!extmem_sizes[x]) x--;
		SendMessage(GetDlgItem(hwnd,IDC_RAM_EXTAMOUNT),CB_SETCURSEL,(WPARAM)x,0L);

		return TRUE;
	}
	if (msg == WM_COMMAND) {
		if (wParam == IDOK) {
			x = GetDlgItemInt(hwnd,IDC_RAM_CONVAMOUNT,NULL,FALSE);
			sprintf(buf,"%u",x);
			WritePrivateProfileString("RAM","ConvSize",buf,inifile);
			x = GetDlgItemInt(hwnd,IDC_RAM_EXTAMOUNT,NULL,FALSE);
			sprintf(buf,"%u",x);
			WritePrivateProfileString("RAM","ExtSize",buf,inifile);

			EndDialog(hwnd,1);
		}
		if (wParam == IDCANCEL) {
			EndDialog(hwnd,0);
		}
	}

	return FALSE;
}

void RamConfig()
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_RAMCONFIG),hwndMain,RamConfigDlg);
	SetFocus(hwndMain);
}
