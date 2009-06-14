/* BIOS driver and handling code

- for purposes of easy design at this stage, this code is part of a patch in
  cpuexec.c and cpudec.c where if the CPU executes a instruction at specific
  areas in ROM, control is passed to the INT routines in this source file.
  For example, INT 13h services are emulated here by this code, transparent to
  the program running in this emulator
*/

#include "global.h"
#include "resource.h"
#include <stdio.h>
#include "addrbus.h"
#include "hardware.h"
#include "cpuexec.h"
#include "ram.h"
#include "fdsim.h"
#include "hdsim.h"
#include "bios.h"
#include "cmos.h"
#include "stackops.h"
#include "lib.h"
#include "direct86.h"

DWORD			addrbus_access_rom(int size,int mod,DWORD dat,DWORD addr);

int eprom_latch=0;

/* ROM access handler (maps ROM[] to F000:0000 in emulation) */
DWORD addrbus_access_rom(int size,int mod,DWORD dat,DWORD addr)
{
	DWORD r;
	int banko,x;

	r=0;

	if (size > 4) size=4;
	if (!size) return 0;

	if (mod == AB_MOD_READ || mod == AB_MOD_SNAPSHOT) {
		addr += size-1;
		banko = (int)(addr&0xFFFF);

		for (x=0;x < size;x++) {
			r = (r<<8) | ((DWORD)ROM[banko]);

			if (!banko) {
				banko=0xFFFF;
			}
			else {
				banko--;
			}
		}
	}

	if (mod == AB_MOD_WRITE || mod == AB_MOD_SNAPSHOTWRITE) {
		if (eprom_latch) {
			banko = (int)(addr&0xFFFF);

			for (x=0;x < size;x++) {
				ROM[banko] = (BYTE)(dat&0xFF);
				dat >>= 8;

				if (!banko) {
					banko=0xFFFF;
				}
				else {
					banko++;
				}
			}
		}
	}

	return r;
}

/* BIOS housekeeping */
int				bios_power=0;
int				bios_keyboard_trd;
DWORD			clock_count=0,clock_count_base=0;
char			biosint13_diskiobuf[65536*2];
BOOL			ROMFlash=FALSE;
BYTE*			ROM=NULL;
char			ROMfilepath[1024];
int				bios_drive_a_type=0;
int				bios_drive_a_heads=0;
int				bios_drive_a_sectors=0;
int				bios_drive_a_tracks=0;
int				bios_drive_b_type=0;
int				bios_drive_b_heads=0;
int				bios_drive_b_sectors=0;
int				bios_drive_b_tracks=0;
int				bios_drive_c_type=0;
int				bios_drive_d_type=0;
floppydrv*		bios_drive_a=NULL;
floppydrv*		bios_drive_b=NULL;
harddiskdrv*	bios_drive_c=NULL;
harddiskdrv*	bios_drive_d=NULL;
char			bios_tmp[1024];

/* 0 = none specified
   1 = 360Kb, 5.25 in
   2 = 1.2Mb, 5.25 in
   3 = 720Kb, 3.5 in
   4 = 1.44Mb, 3.5 in
   5 = 2.88Mb, 3.5 in */

char *bios_drive_fd_types[] = {
	"None",
	"360KB (5.25 inch)",
	"1.2MB (5.25 inch)",
	"720KB (3.5 inch)",
	"1.44MB (3.5 inch)",
	"2.88MB (3.5 inch)",
	NULL};

/* diskette parameter table data to be stored in RAM */
BYTE bios_drive_parmtabl[6][11] = {
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},       /* typical nonexistient drive parameters */
    {0xDF,0x02,0x25,0x02,0x09,0x2A,0xFF,0x50,0xF6,0x15,0x08},       /* typical 360K drive parameters */
    {0xDF,0x02,0x25,0x02,0x0F,0x1B,0xFF,0x54,0xF6,0x15,0x08},       /* typical 1.2M drive parameters */
    {0xAF,0x02,0x25,0x02,0x09,0x2A,0xFF,0x50,0xF6,0x15,0x08},       /* typical 720K drive parameters */
    {0xAF,0x02,0x25,0x02,0x12,0x1B,0xFF,0x6C,0xF6,0x15,0x08},       /* typical 1.44M drive parameters */
    {0xAF,0x02,0x25,0x02,0x24,0x1B,0xFF,0x50,0xF6,0x15,0x08},       /* typical 2.88M drive parameters */
};

char *bios_drive_hd_types[] = {
	"None",
	"360KB (floppy as HD)",
	"1.2MB (floppy as HD)",
	"720KB (floppy as HD)",
	"1.44MB (floppy as HD)",
	"2.88MB (floppy as HD)",
	NULL};

void SelectBIOSvfd(HWND parent,int drv)
{
	OPENFILENAME ofn;
	char *filterstr = " Virtual Floppy Disks\x00*.vfd\x00 all files\x00*.*\x00\x00\x00\x00";
	char *filterstr2 = " Virtual Hard Disks\x00*.vhd\x00 Virtual Floppy Disks\x00*.vfd\x00 all files\x00*.*\x00\x00\x00\x00";
	char *title = "Select virtual floppy file";
	char filename[1024];

	filename[0] = 0;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = parent;
	ofn.hInstance = hInst;
	if (drv >= 2)		ofn.lpstrFilter = filterstr2;
	else				ofn.lpstrFilter = filterstr;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 1000;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = title;
#ifdef WIN95
	ofn.Flags = OFN_HIDEREADONLY | OFN_LONGNAMES | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
#else
	ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
#endif
	ofn.lpstrDefExt = NULL;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lCustData = 0;
	ofn.lpfnHook = 0;
	ofn.lpTemplateName = NULL;

	if (GetOpenFileName(&ofn)) {
		if (drv == 0) {
			if (!bios_drive_a) bios_drive_a = floppydrv_alloc();
			if (floppydrv_selectfile(bios_drive_a,filename)) WritePrivateProfileString("drv floppy0","file",filename,common_ini_file);
		}
		if (drv == 1) {
			if (!bios_drive_b) bios_drive_b = floppydrv_alloc();
			if (floppydrv_selectfile(bios_drive_b,filename)) WritePrivateProfileString("drv floppy1","file",filename,common_ini_file);
		}
		if (drv == 2) {
			if (!bios_drive_c) bios_drive_c = harddiskdrv_alloc();
			if (harddiskdrv_selectfile(bios_drive_c,filename)) WritePrivateProfileString("drv hd0","file",filename,common_ini_file);
		}
		if (drv == 3) {
			if (!bios_drive_d) bios_drive_d = harddiskdrv_alloc();
			if (harddiskdrv_selectfile(bios_drive_d,filename)) WritePrivateProfileString("drv hd1","file",filename,common_ini_file);
		}
	}
}

void BIOSPutConfig(HWND hwnd,int id,floppydrv *t)
{
	char buf[512];

	sprintf(buf,"%u sects, %u heads, %u tracks",t->sectors,t->heads,t->tracks);

	SetDlgItemText(hwnd,id,buf);
}

void BIOSPutConfigHD(HWND hwnd,int id,harddiskdrv *t)
{
	char buf[512];

	sprintf(buf,"%u sects, %u heads, %u tracks",t->sectors,t->heads,t->tracks);

	SetDlgItemText(hwnd,id,buf);
}

BOOL CALLBACK DlgBIOSConfig(HWND hwnd,UINT iMessage,WPARAM wParam,LPARAM lParam) {
	char buf[64];
	int wID,x;
	HWND ctr;

	if (iMessage == WM_INITDIALOG) {
		ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_A);
		for (x=0;bios_drive_fd_types[x] != NULL;x++)
			SendMessage(ctr,CB_ADDSTRING,0,(DWORD)bios_drive_fd_types[x]);

		SendMessage(ctr,CB_SETCURSEL,(WPARAM)bios_drive_a_type,0);

		ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_B);
		for (x=0;bios_drive_fd_types[x] != NULL;x++)
			SendMessage(ctr,CB_ADDSTRING,0,(DWORD)bios_drive_fd_types[x]);

		SendMessage(ctr,CB_SETCURSEL,(WPARAM)bios_drive_b_type,0);

		ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_C);
		for (x=0;bios_drive_hd_types[x] != NULL;x++)
			SendMessage(ctr,CB_ADDSTRING,0,(DWORD)bios_drive_hd_types[x]);

		SendMessage(ctr,CB_SETCURSEL,(WPARAM)bios_drive_c_type,0);

		ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_D);
		for (x=0;bios_drive_hd_types[x] != NULL;x++)
			SendMessage(ctr,CB_ADDSTRING,0,(DWORD)bios_drive_hd_types[x]);

		SendMessage(ctr,CB_SETCURSEL,(WPARAM)bios_drive_d_type,0);

		BIOSPutConfig(hwnd,IDC_BIOSCFG_DRIVE_A_CONFIGSTR,bios_drive_a);
		BIOSPutConfig(hwnd,IDC_BIOSCFG_DRIVE_B_CONFIGSTR,bios_drive_b);
		BIOSPutConfigHD(hwnd,IDC_BIOSCFG_DRIVE_C_CONFIGSTR,bios_drive_c);
		BIOSPutConfigHD(hwnd,IDC_BIOSCFG_DRIVE_D_CONFIGSTR,bios_drive_d);

		return TRUE;
	}
	else if (iMessage == WM_COMMAND) {
		wID = (int)wParam;
		if (wID == IDC_BIOSCFG_DRIVE_A_SELFILE) {
			SelectBIOSvfd(hwnd,0);
		}
		if (wID == IDC_BIOSCFG_DRIVE_A_SELNOFILE) {
			if (!bios_drive_a) bios_drive_a = floppydrv_alloc();
			floppydrv_selectfile(bios_drive_a,NULL);
		}
		if (wID == IDC_BIOSCFG_DRIVE_B_SELFILE) {
			SelectBIOSvfd(hwnd,1);
		}
		if (wID == IDC_BIOSCFG_DRIVE_B_SELNOFILE) {
			if (!bios_drive_b) bios_drive_b = floppydrv_alloc();
			floppydrv_selectfile(bios_drive_b,NULL);
		}
		if (wID == IDC_BIOSCFG_DRIVE_C_SELFILE) {
			SelectBIOSvfd(hwnd,2);
		}
		if (wID == IDC_BIOSCFG_DRIVE_C_SELNOFILE) {
			if (!bios_drive_c) bios_drive_c = harddiskdrv_alloc();
			harddiskdrv_selectfile(bios_drive_c,NULL);
		}
		if (wID == IDC_BIOSCFG_DRIVE_D_SELFILE) {
			SelectBIOSvfd(hwnd,3);
		}
		if (wID == IDC_BIOSCFG_DRIVE_D_SELNOFILE) {
			if (!bios_drive_d) bios_drive_d = harddiskdrv_alloc();
			harddiskdrv_selectfile(bios_drive_d,NULL);
		}
		if (wID == IDOK) {
			ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_A);
			bios_drive_a_type = SendMessage(ctr,CB_GETCURSEL,0,0);
			ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_B);
			bios_drive_b_type = SendMessage(ctr,CB_GETCURSEL,0,0);
			ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_C);
			bios_drive_c_type = SendMessage(ctr,CB_GETCURSEL,0,0);
			ctr=GetDlgItem(hwnd,IDC_BIOSCFG_DRIVE_D);
			bios_drive_d_type = SendMessage(ctr,CB_GETCURSEL,0,0);

			sprintf(buf,"%d",bios_drive_a_type); WritePrivateProfileString("drv floppy0","type",buf,common_ini_file);
			sprintf(buf,"%d",bios_drive_b_type); WritePrivateProfileString("drv floppy1","type",buf,common_ini_file);
			sprintf(buf,"%d",bios_drive_c_type); WritePrivateProfileString("drv hd0","type",buf,common_ini_file);
			sprintf(buf,"%d",bios_drive_d_type); WritePrivateProfileString("drv hd1","type",buf,common_ini_file);

			EndDialog(hwnd,0);
		}
		if (wID == IDCANCEL) {
			EndDialog(hwnd,0);
		}

		BIOSPutConfig(hwnd,IDC_BIOSCFG_DRIVE_A_CONFIGSTR,bios_drive_a);
		BIOSPutConfig(hwnd,IDC_BIOSCFG_DRIVE_B_CONFIGSTR,bios_drive_b);
		BIOSPutConfigHD(hwnd,IDC_BIOSCFG_DRIVE_C_CONFIGSTR,bios_drive_c);
		BIOSPutConfigHD(hwnd,IDC_BIOSCFG_DRIVE_D_CONFIGSTR,bios_drive_d);
	}
	return FALSE;
}

void ConfigureBIOS()
{
#ifdef WIN95
	DialogBox(hInst,MAKEINTRESOURCE(IDD_BIOSCFG),hwndMain,(int (__stdcall *)(struct HWND__ *,unsigned int,unsigned int,long))DlgBIOSConfig);
#else
	DialogBox(hInst,MAKEINTRESOURCE(IDD_BIOSCFG),hwndMain,DlgBIOSConfig);
#endif
	SetFocus(hwndMain);
}

DWORD bios_eprom_latch(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)eprom_latch;
	}

	if (dir == IO_PORT_OUT) {
		eprom_latch = (int)data;
	}

	return r;
}

void ResetBIOS()
{
	int x;
	DWORD o;
	FILE *fp;

	memset(ROM,0xCF,65536);
	eprom_latch=0;

	assign_IO_port(0xCC,bios_eprom_latch);

	fp=fopen(ROMfilepath,"rb");
	if (fp) {
		fread(ROM,65536,1,fp);
		fclose(fp);

		if (bios_power) {
			for (o=0xF0000;o < 0x100000;o += 0x1000) addrbus_make_page_adaptor(o,addrbus_access_rom);
			for (o=0xFFFF0000;o != 0;o += 0x1000) addrbus_make_page_adaptor(o,addrbus_access_rom);
		}
	}
	else {
		MessageBox(hwndMain,"WARNING: Missing ROM BIOS image ROM\\MAIN.ROM! Some parts of the BIOS will not be functional.","",MB_OK);
	}

/* set up interrupt vector table and associated routines */
	for (x=0x00;x <= 0xFF;x++) writememdwordlinear(x*4,0xF0000000);

    writememdwordlinear(0x00,0xF000EC00);   /* INT 00h */
    writememdwordlinear(0x24,0xF000AB09);   /* INT 09h (keyboard interrupt) */
    writememdwordlinear(0x40,0xF000AB10);   /* INT 10h */
    writememdwordlinear(0x44,0xF000AB11);   /* INT 11h */
    writememdwordlinear(0x48,0xF000AB12);   /* INT 12h */
    writememdwordlinear(0x4C,0xF000AB13);   /* INT 13h */
    writememdwordlinear(0x50,0xF000AB14);   /* INT 14h */
    writememdwordlinear(0x54,0xF000AB15);   /* INT 15h */
    writememdwordlinear(0x58,0xF000AB16);   /* INT 16h */
    writememdwordlinear(0x5C,0xF000AB17);   /* INT 17h */
    writememdwordlinear(0x64,0xF000AB19);   /* INT 19h */
    writememdwordlinear(0x68,0xF000AB1A);   /* INT 1Ah */

	if (!bios_drive_a)
		bios_drive_a = floppydrv_alloc();
	if (!bios_drive_b)
		bios_drive_b = floppydrv_alloc();
	if (!bios_drive_c)
		bios_drive_c = harddiskdrv_alloc();
	if (!bios_drive_d)
		bios_drive_d = harddiskdrv_alloc();

	if (bios_drive_a_type == 0) {
		bios_drive_a_heads = 0;
		bios_drive_a_sectors = 0;
		bios_drive_a_tracks = 0;
	}
	if (bios_drive_a_type == 1) {
		bios_drive_a_heads = 2;
		bios_drive_a_sectors = 9;
		bios_drive_a_tracks = 40;
	}
	if (bios_drive_a_type == 2) {
		bios_drive_a_heads = 2;
		bios_drive_a_sectors = 15;
		bios_drive_a_tracks = 40;
	}
	if (bios_drive_a_type == 3) {
		bios_drive_a_heads = 2;
		bios_drive_a_sectors = 9;
		bios_drive_a_tracks = 80;
	}
	if (bios_drive_a_type == 4) {
		bios_drive_a_heads = 2;
		bios_drive_a_sectors = 18;
		bios_drive_a_tracks = 80;
	}
	if (bios_drive_a_type == 5) {
		bios_drive_a_heads = 2;
		bios_drive_a_sectors = 36;
		bios_drive_a_tracks = 80;
	}

	for (x=0;x < 11;x++) writemembytelinear(0x510+x,bios_drive_parmtabl[bios_drive_a_type][x]);
	writememdwordlinear(0x78,0x00000510);

	if (bios_drive_b_type == 0) {
		bios_drive_b_heads = 0;
		bios_drive_b_sectors = 0;
		bios_drive_b_tracks = 0;
	}
	if (bios_drive_b_type == 1) {
		bios_drive_b_heads = 2;
		bios_drive_b_sectors = 9;
		bios_drive_b_tracks = 40;
	}
	if (bios_drive_b_type == 2) {
		bios_drive_b_heads = 2;
		bios_drive_b_sectors = 15;
		bios_drive_b_tracks = 40;
	}
	if (bios_drive_b_type == 3) {
		bios_drive_b_heads = 2;
		bios_drive_b_sectors = 9;
		bios_drive_b_tracks = 80;
	}
	if (bios_drive_b_type == 4) {
		bios_drive_b_heads = 2;
		bios_drive_b_sectors = 18;
		bios_drive_b_tracks = 80;
	}
	if (bios_drive_b_type == 5) {
		bios_drive_b_heads = 2;
		bios_drive_b_sectors = 36;
		bios_drive_b_tracks = 80;
	}

/* fill in BIOS data area */
	writememwordfarptr(0x40,0x13,640);

/* make a template data area */
	writememwordfarptr(0x40,0x00,0x3F8);	// COM1 address
	writememwordfarptr(0x40,0x02,0x2F8);	// COM2 address
	writememwordfarptr(0x40,0x04,0x000);	// COM3 address
	writememwordfarptr(0x40,0x06,0x000);	// COM4 address
	writememwordfarptr(0x40,0x08,0x378);	// LPT1 address
	writememwordfarptr(0x40,0x0A,0x278);	// LPT2 address
	writememwordfarptr(0x40,0x0C,0x000);	// LPT3 address
	writememwordfarptr(0x40,0x0E,0x000);	// LPT4 address

/* "installed hardware 1" */
    writemembytefarptr(0x40,0x10,(0x1 << 6) |   /* (2-bit) 2 floppy drives */
        (2 << 4) |                              /* (2-bit) 80x25 color mode (0=EGA/VGA mode, 1=40x25 color, 2=80x25 color, 3=80x25 mono) */
        (0 << 3) |                              /* (1-bit) RESERVED */
        (0 << 2) |                              /* (1-bit) no "pointing device" built in (PC/XT and later) */
        (1 << 1) |                              /* (1-bit) math coprocessor installed (not PCjr, PS/1, Convertible) */
        (0 << 0));                              /* (1-bit) floppy drive installed for boot */

/* "installed hardware 2" */
    writemembytefarptr(0x40,0x11,(0x2 << 6) |   /* (2-bit) # of printer adaptors */
        (0 << 5) |                              /* (1-bit) Internal modem (Convertible PCs) */
        (0 << 4) |                              /* (1-bit) Joystick installed (PS/1) */
        (2 << 1) |                              /* (3-bit) # of RS-232 adaptors (COM ports) */
        (1 << 0));                              /* (1-bit) RESERVED or "DMA device installed" (PCjr) */

    writemembytefarptr(0x40,0x12,0x00);         /* power-on self test status (Convertible PCs) */
    writememwordfarptr(0x40,0x13,640);          /* memory size (in K) */
    writememwordfarptr(0x40,0x15,0);            /* Manufacturing test port (Phoenix AT) */
    writemembytefarptr(0x40,0x17,0);            /* keyboard control 1 */
    writemembytefarptr(0x40,0x18,0);            /* keyboard control 2 */
    writemembytefarptr(0x40,0x19,0);            /* alternate keypad entry */
    writememwordfarptr(0x40,0x1A,0x1E);         /* circular keyboard buffer head pointer */
    writememwordfarptr(0x40,0x1C,0x1E);         /* circular keyboard buffer tail pointer */

	writemembytefarptr(0x40,0x3E,0x00);			// floppy recalibrate status

/* floppy motor status */
    writemembytefarptr(0x40,0x3F,(0 << 7) |     /* (1-bit) interrupt flag */
        (0 << 6) |                              /* (1-bit) RESERVED */
        (0 << 4) |                              /* (2-bit) drive selected */
        (0 << 3) |                              /* (1-bit) drive 3 motor ON */
        (0 << 2) |                              /* (1-bit) drive 2 motor ON */
        (0 << 1) |                              /* (1-bit) drive 1 motor ON */
        (0 << 0));                              /* (1-bit) drive 0 motor ON */

    writemembytefarptr(0x40,0x40,0x00);         /* floppy motor off counter */

/* floppy previous operation status */
	writemembytefarptr(0x40,0x41,(0 << 7) |		// (1-bit) 1=drive not ready
        (0 << 6) |                              // (1-bit) 1=seek operation failed
        (0 << 5) |                              // (1-bit) 1=general controller failure
        (0 << 4) |                              // (1-bit) 1=CRC error on diskette read
        (0 << 3) |                              // (1-bit) 1=DMA overrun on operation
        (0 << 2) |                              // (1-bit) 1=requested sector not found
        (0 << 1) |                              // (1-bit) 1=address mark not found
        (0 << 0));                              // (1-bit) 1=invalid drive parameter
// According to the documentation, in the lower 4 bits more status info can be read:
// 0011=write protect error
// 0110=disk changed (door opened)
// 1001=DMA attempt across 64K segment boundary
// 1100=media type not found

// floppy controller status bytes
	writemembytefarptr(0x40,0x42,0xC0);			
	writemembytefarptr(0x40,0x43,0x00);
	writemembytefarptr(0x40,0x44,0x00);
	writemembytefarptr(0x40,0x45,0x00);
	writemembytefarptr(0x40,0x46,0x00);
	writemembytefarptr(0x40,0x47,0x00);
	writemembytefarptr(0x40,0x48,0x00);

// display mode
	writemembytefarptr(0x40,0x49,0x03);		// 80x25 alphanumeric mode (color)
}

void PowerDownBIOS()
{
	DWORD x;

// unmap ROM from address space
	for (x=0xF0000;x < 0x100000;x += 0x1000) addrbus_make_page_empty(x);
	for (x=0xFFFF0000;x != 0;x += 0x1000) addrbus_make_page_empty(x);

	unassign_IO_port(0xCC);

	bios_power=0;
}

void PowerOnBIOS()
{
	bios_power=1;
	ResetBIOS();
}

int IsBIOSPowered()
{
	return bios_power;
}

void InitBIOS()
{
	char buf[512];

	if (!ROM) ROM = (BYTE*)malloc(65536);

	GetCurrentDirectory(900,ROMfilepath);
	if (ROMfilepath[strlen(ROMfilepath)-1] != '\\') strcat(ROMfilepath,"\\");
    strcat(ROMfilepath,"bios\\main.rom");

	bios_drive_a = floppydrv_alloc();
	buf[0] = 0; GetPrivateProfileString("drv floppy0","file","",buf,511,common_ini_file); if (buf[0]) floppydrv_selectfile(bios_drive_a,buf);

	bios_drive_b = floppydrv_alloc();
	buf[0] = 0; GetPrivateProfileString("drv floppy1","file","",buf,511,common_ini_file); if (buf[0]) floppydrv_selectfile(bios_drive_b,buf);

	bios_drive_c = harddiskdrv_alloc();
	buf[0] = 0; GetPrivateProfileString("drv hd0","file","",buf,511,common_ini_file); if (buf[0]) harddiskdrv_selectfile(bios_drive_c,buf);

	bios_drive_d = harddiskdrv_alloc();
	buf[0] = 0; GetPrivateProfileString("drv hd1","file","",buf,511,common_ini_file); if (buf[0]) harddiskdrv_selectfile(bios_drive_d,buf);

	bios_drive_a_type=GetPrivateProfileInt("drv floppy0","type",0,common_ini_file);
	bios_drive_b_type=GetPrivateProfileInt("drv floppy1","type",0,common_ini_file);
	bios_drive_c_type=GetPrivateProfileInt("drv hd0","type",0,common_ini_file);
	bios_drive_d_type=GetPrivateProfileInt("drv hd1","type",0,common_ini_file);

	bios_power=0;
}

void CloseBIOS()
{
	if (bios_drive_a) floppydrv_free(bios_drive_a);
	if (bios_drive_b) floppydrv_free(bios_drive_b);
	if (bios_drive_c) harddiskdrv_free(bios_drive_c);
	if (bios_drive_d) harddiskdrv_free(bios_drive_d);

	bios_drive_a=NULL;
	bios_drive_b=NULL;
	bios_drive_c=NULL;
	bios_drive_d=NULL;

	if (ROM) free(ROM);
	ROM=NULL;
	bios_power=0;

	PowerDownBIOS();
}

void BIOS_Int10()
{
	int cmd;
	char c;

	cmd = (ireg_eax >> 8) & 0xFF;

	if (cmd == 0x0E) {
		c = (char)(ireg_eax & 0xFF);
	}
	else if (cmd == 0x0F) {
		ireg_eax &= ~0xFFFF;
		ireg_eax |= 0x5003;
		ireg_ebx &= ~0xFFFF;
	}
// 12h - Alternate Select (???)
	else if (cmd == 0x12) {
// 10h - return config info?
		if ((ireg_ebx & 255) == 0x10) {
			ireg_ebx = 0;
			ireg_ecx = 0;
		}
		else {
//			MessageBox(hwndMain,"Unknown INT 10h AH=12h call","",MB_OK);
		}
	}
// 1Ah - Display code stuff
	else if (cmd == 0x1A) {
// read display codes?
		if ((ireg_eax & 255) == 0) {
			ireg_eax = 0x1A;
			ireg_ebx = 0x0101;
		}
		else {
//			MessageBox(hwndMain,"Unknown INT 10h AH=1Ah call","",MB_OK);
		}
	}
	else {
//		MessageBox(hwndMain,"Unknown INT 10h call","",MB_OK);
//		code_freerun=FALSE;
		ireg_eax = 0xFF00;
	}

	InterruptFrameRet16();
}

// BIOS equipment check
void BIOS_Int11()
{
	ireg_eax = 0xC427;

	InterruptFrameRet16();
}

void BIOS_Int13()
{
	int cmd,drv,num,x,seg,off,s,o,ptri;
	int numdrv;
	WORD w1,w2,w3;

	cmd = (ireg_eax >> 8) & 0xFF;
// reset disk system?
	if (cmd == 0) {
		ireg_eax = 0;		// no error
	}
// read disk?
	else if (cmd == 2) {
		num = ireg_eax & 0xFF;

// simulate BIOS finickiness about DMA overruns
		if (((ireg_ebx & 0xFFFF)+(num<<9)) > 0x10000) {
			ireg_eax = 0x0800;
		}
		else {
			drv = ireg_edx & 0xFF;
			if (drv == 0 && bios_drive_a) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ireg_eax = floppydrv_read(bios_drive_a,ireg_ecx & 0xFF,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 8) & 0xFF,&num,biosint13_diskiobuf) << 8;
				if (!ireg_eax) {
					ptri = (seg<<4)+off;
					for (x=0;x < (512*num);x++) hardwritemembyte(ptri++,biosint13_diskiobuf[x]);
				}
				ireg_eax |= (num&0xFF);
			}
			else if (drv == 1 && bios_drive_b) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ireg_eax = floppydrv_read(bios_drive_b,ireg_ecx & 0xFF,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 8) & 0xFF,&num,biosint13_diskiobuf) << 8;
				if (!ireg_eax) {
					ptri = (seg<<4)+off;
					for (x=0;x < (512*num);x++) hardwritemembyte(ptri++,biosint13_diskiobuf[x]);
				}
				ireg_eax |= (num&0xFF);
			}
			else if (drv == 0x80 && bios_drive_c->diskindrive) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ireg_eax = harddiskdrv_read(bios_drive_c,ireg_ecx & 0x3F,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 6) & 0x3FF,&num,biosint13_diskiobuf) << 8;
				if (!ireg_eax) {
					ptri = (seg<<4)+off;
					for (x=0;x < (512*num);x++) hardwritemembyte(ptri++,biosint13_diskiobuf[x]);
				}
//				ireg_eax |= (num&0xFF);   somehow it's standard the BIOS never returns how many sectors you read for hard drives
			}
			else if (drv == 0x81 && bios_drive_d->diskindrive) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ireg_eax = harddiskdrv_read(bios_drive_d,ireg_ecx & 0x3F,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 6) & 0x3FF,&num,biosint13_diskiobuf) << 8;
				if (!ireg_eax) {
					ptri = (seg<<4)+off;
					for (x=0;x < (512*num);x++) hardwritemembyte(ptri++,biosint13_diskiobuf[x]);
				}
//				ireg_eax |= (num&0xFF);   somehow it's standard the BIOS never returns how many sectors you read for hard drives
			}
			else {
				ireg_eax = 0xAA00;
			}
		}
	}
// write disk?
	else if (cmd == 3) {
		num = ireg_eax & 0xFF;

// simulate BIOS finickiness about DMA overruns
		if (((ireg_ebx & 0xFFFF)+(num<<9)) > 0x10000) {
			ireg_eax = 0x0800;
		}
		else {
			drv = ireg_edx & 0xFF;
			if (drv == 0 && bios_drive_a) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ptri = (seg<<4)+off;
				for (x=0;x < (512*num);x++) biosint13_diskiobuf[x] = hardmembyte(ptri++);
				ireg_eax = floppydrv_write(bios_drive_a,ireg_ecx & 0xFF,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 8) & 0xFF,&num,biosint13_diskiobuf) << 8;
				ireg_eax |= (num&0xFF);
			}
			else if (drv == 1 && bios_drive_b) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ptri = (seg<<4)+off;
				for (x=0;x < (512*num);x++) biosint13_diskiobuf[x] = hardmembyte(ptri++);
				ireg_eax = floppydrv_write(bios_drive_b,ireg_ecx & 0xFF,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 8) & 0xFF,&num,biosint13_diskiobuf) << 8;
				ireg_eax |= (num&0xFF);
			}
			else if (drv == 0x80 && bios_drive_c->diskindrive) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ptri = (seg<<4)+off;
				for (x=0;x < (512*num);x++) biosint13_diskiobuf[x] = hardmembyte(ptri++);
				ireg_eax = harddiskdrv_write(bios_drive_c,ireg_ecx & 0x3F,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 6) & 0x3FF,&num,biosint13_diskiobuf) << 8;
//				ireg_eax |= (num&0xFF);   somehow it's standard the BIOS never returns how many sectors you read for hard drives
			}
			else if (drv == 0x81 && bios_drive_d->diskindrive) {
				seg = ireg_es & 0xFFFF;
				off = ireg_ebx & 0xFFFF;
				ptri = (seg<<4)+off;
				for (x=0;x < (512*num);x++) biosint13_diskiobuf[x] = hardmembyte(ptri++);
				ireg_eax = harddiskdrv_write(bios_drive_d,ireg_ecx & 0x3F,(ireg_edx >> 8) & 0xFF,(ireg_ecx >> 6) & 0x3FF,&num,biosint13_diskiobuf) << 8;
//				ireg_eax |= (num&0xFF);   somehow it's standard the BIOS never returns how many sectors you read for hard drives
			}
			else {
				ireg_eax = 0xAA00;
			}
		}
	}
// read drive parameters?
	else if (cmd == 8) {
		drv = ireg_edx & 0xFF;

		numdrv=0;
		if (bios_drive_a_type)			numdrv++;
		if (bios_drive_b_type)			numdrv++;
		if (bios_drive_c->diskindrive)	numdrv++;
		if (bios_drive_d->diskindrive)	numdrv++;

		if (drv == 0 && bios_drive_a) {
			ireg_eax = 0;
			ireg_ebx = bios_drive_a_type;
			ireg_ecx = ((bios_drive_a->tracks-1) << 8) | (bios_drive_a->sectors);
			ireg_edx = ((bios_drive_a->heads-1) << 8) | numdrv;
			ireg_edi = 0x0510;
			ireg_es  = 0x0000;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 &= ~1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
		else if (drv == 1 && bios_drive_b) {
			ireg_eax = 0;
			ireg_ebx = bios_drive_b_type;
			ireg_ecx = ((bios_drive_b->tracks-1) << 8) | (bios_drive_b->sectors);
			ireg_edx = ((bios_drive_b->heads-1) << 8) | numdrv;
			ireg_edi = 0x0520;
			ireg_es  = 0x0000;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 &= ~1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
		else if (drv == 0x80 && bios_drive_c->diskindrive) {
			ireg_eax = 0;
			ireg_ebx = bios_drive_c_type;
			ireg_ecx = ((bios_drive_c->tracks-1) << 8) | ((((bios_drive_c->tracks-1) >> 8)&3) << 6) | (bios_drive_c->sectors&63);
			ireg_edx = ((bios_drive_c->heads-1) << 8) | numdrv;
			ireg_edi = 0x0530;
			ireg_es  = 0x0000;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 &= ~1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
		else if (drv == 0x81 && bios_drive_d->diskindrive) {
			ireg_eax = 0;
			ireg_ebx = bios_drive_d_type;
			ireg_ecx = ((bios_drive_d->tracks-1) << 8) | (((bios_drive_d->tracks-1) >> 8) << 6) | (bios_drive_d->sectors&63);
			ireg_edx = ((bios_drive_d->heads-1) << 8) | numdrv;
			ireg_edi = 0x0540;
			ireg_es  = 0x0000;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 &= ~1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
		else {
			ireg_eax = 0xFF00;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 |= 1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
	}
// read DASD type?
	else if (cmd == 0x15) {
		w1 = (WORD)PopWord();
		w2 = (WORD)PopWord();
		w3 = (WORD)PopWord();
// set flags (w3)
		w3 &= ~1;
		PushWord(w3);
		PushWord(w2);
		PushWord(w1);
		drv = ireg_edx & 0xFF;
		if (drv == 0 && bios_drive_a) {
			ireg_eax = 0x0200;
			ireg_ecx = 0;
			ireg_edx = 0;
		}
		else if (drv == 1 && bios_drive_b) {
			ireg_eax = 0x0200;
			ireg_ecx = 0;
			ireg_edx = 0;
		}
		else if (drv == 2 && bios_drive_c->diskindrive) {
			ireg_eax = 0x0300;
			ireg_ecx = 0;
			ireg_edx = 0;
		}
		else if (drv == 3 && bios_drive_d->diskindrive) {
			ireg_eax = 0x0300;
			ireg_ecx = 0;
			ireg_edx = 0;
		}
		else {
			ireg_eax = 0;
			ireg_ecx = 0;
			ireg_edx = 0;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 |= 1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
	}
// get diskette change line status?
	else if (cmd == 0x16) {
		w1 = (WORD)PopWord();
		w2 = (WORD)PopWord();
		w3 = (WORD)PopWord();
// set flags (w3)
		w3 &= ~1;
		PushWord(w3);
		PushWord(w2);
		PushWord(w1);
		drv = ireg_edx & 0xFF;
		if (drv == 0 && bios_drive_a) {
			ireg_eax = (bios_drive_a->changeline || !bios_drive_a->diskindrive) ? 0x0600 : 0x0000;
			ireg_ecx = 0;
			ireg_edx = 0;

			if (bios_drive_a->changeline || !bios_drive_a->diskindrive) {
				w1 = (WORD)PopWord();
				w2 = (WORD)PopWord();
				w3 = (WORD)PopWord();
// set flags (w3)
				w3 |= 1;
				PushWord(w3);
				PushWord(w2);
				PushWord(w1);
			}
		}
		else if (drv == 1 && bios_drive_b) {
			ireg_eax = (bios_drive_b->changeline || !bios_drive_b->diskindrive) ? 0x0600 : 0x0000;
			ireg_ecx = 0;
			ireg_edx = 0;

			if (bios_drive_b->changeline || !bios_drive_b->diskindrive) {
				w1 = (WORD)PopWord();
				w2 = (WORD)PopWord();
				w3 = (WORD)PopWord();
// set flags (w3)
				w3 |= 1;
				PushWord(w3);
				PushWord(w2);
				PushWord(w1);
			}
		}
		else if (drv == 2 && bios_drive_c->diskindrive) {
			ireg_eax = 0x0100;
			ireg_ecx = 0;
			ireg_edx = 0;
		}
		else if (drv == 3 && bios_drive_d->diskindrive) {
			ireg_eax = 0x0100;
			ireg_ecx = 0;
			ireg_edx = 0;
		}
		else {
			ireg_eax = 0;
			ireg_ecx = 0;
			ireg_edx = 0;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 |= 1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
	}
// unknown command
	else {
//		MessageBox(hwndMain,"Unknown INT 13h call","",MB_OK);
//		code_freerun=FALSE;
		ireg_eax = 0xFF00;
	}

	o = PopWord();
	s = PopWord();
	x = PopWord();

	if (cmd == 2) {
		if (ireg_eax & 0xFF00)
			x |= 1;
		else
			x &= ~1;
	}

	x |= 0x200;			// set the Interrupt flag

	PushWord(x);
	PushWord(s);
	PushWord(o);

	InterruptFrameRet16();
}

void BIOS_Int14()
{
	BYTE cmd;

	cmd = (BYTE)((ireg_eax >> 8) & 255);

// Function 00h - Return sys config parms?
	if (cmd == 0x00) {
		if (1) {		// fake that no ports are out there
			ireg_eax = 0xFFFF;
		}
	}
// unknown command
	else {
//		MessageBox(hwndMain,"Unknown INT 14h call","",MB_OK);
//		code_freerun=FALSE;
		ireg_eax = 0xFF00;
	}

	InterruptFrameRet16();
}

void BIOS_Int15()
{
	BYTE cmd;
	WORD w1,w2,w3;

	cmd = (BYTE)((ireg_eax >> 8) & 255);

// Function 88h - Return extended memory size?
	if (cmd == 0x88) {
		ireg_eax = 0;
	}
// Function C0h - Return sys config parms?
	else if (cmd == 0xC0) {
		if (1) {		// don't return it
			ireg_eax &= ~0xFF00;
			ireg_eax |= 0x8600;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 |= 1;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
	}
// unknown command
	else {
//		MessageBox(hwndMain,"Unknown INT 15h call","",MB_OK);
//		code_freerun=FALSE;
		ireg_eax = 0xFF00;
	}

	InterruptFrameRet16();
}

void BIOS_Int16()
{
	BYTE cmd,v;
	WORD w1,w2,w3;

	cmd = (BYTE)((ireg_eax >> 8) & 255);

// Function 00h - read character?
	if (cmd == 0) {
		if (memwordfarptr(0x40,0x1A) == memwordfarptr(0x40,0x1C)) {
// Since it is the executioneer that called us upon executing address
// F000:AB16 and leaves us the responsibility of going from there, we
// do not change IP and simply return. The Instruction Pointer will
// thus stay in the same position.
			ireg_flags |= 0x200;
			return;
		}
		else {
			w1 = memwordfarptr(0x40,0x1A);
			w2 = memwordfarptr(0x40,w1);
			ireg_eax = (DWORD)w2;
			w1 += 2;
			w1 = ((w1 - 0x1E) & 30) + 0x1E;
			writememwordfarptr(0x40,0x1A,w1);
		}
	}
// Function 01h - get status?
	else if (cmd == 1) {
		if (memwordfarptr(0x40,0x1A) == memwordfarptr(0x40,0x1C)) {
// make like nothing was pressed.
// To indicate this, set AX=0 and ZF=1
			ireg_eax = 0;
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 |= 0x40;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
		else {
// make like something was pressed.
// To indicate this, set AX=char and ZF=0
			ireg_eax = memwordfarptr(0x40,memwordfarptr(0x40,0x1A));
			w1 = (WORD)PopWord();
			w2 = (WORD)PopWord();
			w3 = (WORD)PopWord();
// set flags (w3)
			w3 &= ~0x40;
			PushWord(w3);
			PushWord(w2);
			PushWord(w1);
		}
	}
// Function 02h - read flags?
	else if (cmd == 2) {
// make like nothing was pressed.
// To indicate this, set AX=0
		ireg_eax = 0;
	}
// Function 03h - set typematic rate/delay?
	else if (cmd == 3) {
// make like nothing was pressed.
// To indicate this, set AX=0
		v = (BYTE)(ireg_eax & 255);
		if (v == 5)
			bios_keyboard_trd = (ireg_ebx & 0xFFFF);
		if (v == 6)
			ireg_ebx = (DWORD)(bios_keyboard_trd & 0xFFFF);

		ireg_eax = 0;
	}
// Function 05h - keyboard write?
	else if (cmd == 5) {
// fake a full buffer
// To indicate this, set AL=1
		ireg_eax = 1;
	}
// Function 09h - keyboard functionality?
	else if (cmd == 9) {
// make like nothing is supported
// To indicate this, set AL=0
		ireg_eax = 0;
	}
// Function 10h - extended keyboard read?
	else if (cmd == 16) {
// make like nothing is there
// To indicate this, set AL=0
		ireg_eax = 0;
	}
// Function 11h - extended keyboard status?
	else if (cmd == 17) {
// make like nothing is there
// To indicate this, set AL=0 and ZF=1;
		ireg_eax = 0;
		w1 = (WORD)PopWord();
		w2 = (WORD)PopWord();
		w3 = (WORD)PopWord();
// set flags (w3)
		w3 |= 0x40;
		PushWord(w3);
		PushWord(w2);
		PushWord(w1);
	}
// Function 12h - extended shift status?
	else if (cmd == 18) {
// make like nothing is there
// To indicate this, set AX=0
		ireg_eax = 0;
	}
	else {
//		MessageBox(hwndMain,"Unknown INT 16h call","",MB_OK);
//		code_freerun=FALSE;
		ireg_eax = 0;
	}

	InterruptFrameRet16();
}

void BIOS_Int17()
{
	BYTE cmd;

	cmd = (BYTE)((ireg_eax >> 8) & 255);

// Function 01h - Return sys config parms?
	if (cmd == 0x01) {
		if (1) {		// fake that no ports are out there
			ireg_eax = 0x0800;
		}
	}
// unknown command
	else {
//		MessageBox(hwndMain,"Unknown INT 17h call","",MB_OK);
//		code_freerun=FALSE;
		ireg_eax = 0xFF00;
	}

	InterruptFrameRet16();
}

void BIOS_Int1A()
{
	BYTE cmd;
	SYSTEMTIME st;
	WORD w1,w2,w3;
	DWORD cnt;

	cmd = (BYTE)((ireg_eax >> 8) & 255);

	if (clock_count_base == 0) {
		clock_count_base = (DWORD)frtime(NULL);
	}

// function 00h - read clock count
	if (cmd == 0x00) {
		clock_count=cnt=(((DWORD)frtime(NULL))-clock_count_base);
		ireg_ecx = (DWORD)(cnt>>16);
		ireg_edx = (DWORD)(cnt&0xFFFF);
		ireg_eax = 0;
		w1 = (WORD)PopWord();
		w2 = (WORD)PopWord();
		w3 = (WORD)PopWord();
// set flags (w3)
		w3 &= ~1;
		PushWord(w3);
		PushWord(w2);
		PushWord(w1);
	}
// function 01h - set clock count
	else if (cmd == 0x01) {
		clock_count_base = ((DWORD)frtime(NULL)) - (((DWORD)(ireg_edx&0xFFFF)) | (((DWORD)(ireg_ecx&0xFFFF))<<16));
		clock_count = 0;
		w1 = (WORD)PopWord();
		w2 = (WORD)PopWord();
		w3 = (WORD)PopWord();
// set flags (w3)
		w3 &= ~1;
		PushWord(w3);
		PushWord(w2);
		PushWord(w1);
	}
// function 02h - read clock time
	else if (cmd == 0x02) {
// return the simulated clock time
		ireg_eax = 0;
		w1 = (WORD)PopWord();
		w2 = (WORD)PopWord();
		w3 = (WORD)PopWord();
// set flags (w3)
		w3 &= ~1;
		PushWord(w3);
		PushWord(w2);
		PushWord(w1);
// BCD hour
		ireg_eax = CMOSmemory[4];
		ireg_ecx = ireg_eax<<8;
// BCD minute
		ireg_ecx |= CMOSmemory[2];
// BCD seconds
		ireg_edx = CMOSmemory[0]<<8;
// Daylights saving (say it's inoperative)
	}
// function 04h - read clock date
	else if (cmd == 0x04) {
// return the actual time
		ireg_eax = 0;
		w1 = (WORD)PopWord();
		w2 = (WORD)PopWord();
		w3 = (WORD)PopWord();
// set flags (w3)
		w3 &= ~1;
		PushWord(w3);
		PushWord(w2);
		PushWord(w1);
// BCD century (for whatever reason)
		ireg_ecx = ((CMOSmemory[9] >= 0xA0) ? 0x20 : 0x19) << 8;
// BCD year
		ireg_ecx |= CMOSmemory[9];
// BCD month
		ireg_edx = CMOSmemory[8]<<8;
// BCD day
		ireg_edx |= CMOSmemory[7];
	}
// function 0Ah - count of days > 1/1/80 (whatever)
	else if (cmd == 0x0A) {
		GetSystemTime(&st);
		ireg_ecx = (st.wYear - 1980);
		ireg_ecx *= ((365 + (st.wYear/4)) - ((st.wYear - 1600)/400));
		ireg_ecx += (st.wMonth >  1)?31:0;
		ireg_ecx += (st.wMonth >  2)?(((st.wYear & 4) == 0) ? 29 : 28):0;
		ireg_ecx += (st.wMonth >  3)?31:0;
		ireg_ecx += (st.wMonth >  4)?30:0;
		ireg_ecx += (st.wMonth >  5)?31:0;
		ireg_ecx += (st.wMonth >  6)?30:0;
		ireg_ecx += (st.wMonth >  7)?31:0;
		ireg_ecx += (st.wMonth >  8)?31:0;
		ireg_ecx += (st.wMonth >  9)?30:0;
		ireg_ecx += (st.wMonth > 10)?31:0;
		ireg_ecx += (st.wMonth > 11)?30:0;
		ireg_ecx += st.wDay;
		ireg_ecx &= 0xFFFF;
	}
// unknown command
	else {
//		MessageBox(hwndMain,"Unknown INT 1Ah call","",MB_OK);
//		ireg_eax = 0xFF00;
		code_freerun=FALSE;
	}

	InterruptFrameRet16();
}
