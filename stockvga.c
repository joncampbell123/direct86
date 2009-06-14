// this code emulates the functionality of your typical
// stock-VGA display hardware
// 
// CAUTION: Some parts of this source may look weird.
//          If they do, it is because your editor uses a
//          different character set than the traditional
//          8-bit extended ASCII character set used in
//          PC VGA alphanumeric display modes.

#include "global.h"
#include "resource.h"
#include <stdio.h>
#include "direct86.h"
#include "stockvga.h"
#include "addrbus.h"
#include "hardware.h"
#include "cpuexec.h"
#include "kbdwin.h"

DWORD vga_crtc_3D5h(int size,int dir,DWORD data);
void VGARenderBlank(int x1,int y1,int x2,int y2);
void VGARender8Walpha(int x1,int y1,int x2,int y2);
void VGARender320x200x256(int x1,int y1,int x2,int y2);

char*						szClassNameVGA = "JMC_PROG_SOTHERE_DEDITx86_VGA";
BOOL						display_win_showing;
HDC							VGADC;
HPALETTE					VGAPaletteObj;
LOGPALETTE*					VGAPaletteLog;
BITMAPINFO*					VGAParams;
BYTE*						VGARenderBuffer;
BYTE*						VGARAM;
int							VGARAMSize;
int							VGAScreenX,VGAScreenY;
int							VGARenderScaleX,VGARenderScaleY;		// fixed point
double						VGA_AspectRatio = (640.0 / 400.0);
BOOL						VGA_TextDisplay=TRUE;
int							VGA_TextFontHeight=16;
DWORD						VGA_OffsetBank = 0xB8000;
int							VGA_UpdateLine[480];
int							VGA_screenwidth,VGA_screenheight;
int							VGA_Cursor_X=0,VGA_Cursor_Y=0;
int							VGA_Cursor_PX=-1,VGA_Cursor_PY=-1;
int							VGA_Cursor_visible=1,VGA_Cursor_visblink=0,VGA_Cursor_visdraw=0;
int							VGA_pan_x,VGA_pan_y;
BYTE						VGAPalette[768];
BYTE						VGACharacterRAM[256*32];
BYTE						BitRev[256];
BYTE						VGA_CRTCREGS[256];
int							VGA_CRTCIndx=0;
int							VGA_powered=0;
int							VGA_cursblinkdelay=0;
HWND						hwndMainVGA;
int							VGA_mode=0;
int							VGA_lines=0;
int							VGA_Palregidx=0;
void						(*vga_render)(int,int,int,int);	// callback for rendering
void						(*vga_kludge)(int,int,int,int);	// callback for "kludge extensions"

void VGARenderByRealDisplay(int x1,int y1,int x2,int y2)
{
	if (x1 < 0)						x1=0;
	if (y1 < 0)						y1=0;
	if (x2 > VGA_screenwidth)		x2=VGA_screenwidth;
	if (y2 > VGA_screenheight)		y2=VGA_screenheight;

	x2--; y2--;
	if (x1 > x2 || y1 > y2) return;

	vga_render(x1,y1,x2,y2);
}

void VGARedraw(int x1,int y1,int x2,int y2)
{
	int h,w;

	if (!VGAParams) return;

	if (x1 < 0) x1=0;
	if (y1 < 0) y1=0;
	if (x2 > VGAScreenX)			x2 = VGAScreenX;
	if (y2 > VGAScreenY)			y2 = VGAScreenY;
	if (y2 > VGA_screenheight) {
		SelectObject(VGADC,GetStockObject(BLACK_BRUSH));
		SelectObject(VGADC,GetStockObject(NULL_PEN));
		Rectangle(VGADC,0,VGA_screenheight,x2+1,y2+1);
		y2 = VGA_screenheight;
	}
	if (x2 > VGA_screenwidth) {
		SelectObject(VGADC,GetStockObject(BLACK_BRUSH));
		SelectObject(VGADC,GetStockObject(NULL_PEN));
		Rectangle(VGADC,VGA_screenwidth,0,x2+1,VGA_screenheight+1);
		x2 = VGA_screenwidth;
	}

	x2--; y2--;
	if (x1 > x2 || y1 > y2) return;

	h = abs(y2-y1)+1;
	w = VGA_screenwidth;
	VGAParams->bmiHeader.biHeight = -h;
	VGAParams->bmiHeader.biWidth = w;
	VGAParams->bmiHeader.biSizeImage = w*h*3;
	SetDIBitsToDevice(VGADC,x1,y1,abs(x2-x1)+1,abs(VGAParams->bmiHeader.biHeight),0,0,0,abs(VGAParams->bmiHeader.biHeight),
		VGARenderBuffer+(((w*y1)+x1)*3),VGAParams,DIB_PAL_COLORS);

	VGAParams->bmiHeader.biHeight = -VGAScreenY;
}

// used by code that is dependant on rendering stuff specific to the VGA screen
void VGARenderByVGA(int x1,int y1,int x2,int y2)
{
	if (!VGAParams || !VGARenderBuffer) return;

	x1 -= VGA_pan_x;
	x2 -= VGA_pan_x;
	y1 -= VGA_pan_y;
	y2 -= VGA_pan_y;

	VGARenderByRealDisplay(x1,y1,x2,y2);
	VGARedraw(x1,y1,x2,y2);
}

long FAR PASCAL MainWndProcVGA(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	int x,y,w,h,t,y2;
	BYTE b1;
	RECT hwndRec;
#ifdef WIN95
	int rw,rh;
	LPRECT rct;
#endif WIN95
	FILE *fp;
	BYTE buffer[256*16];

	switch (iMessage) {
	
	case WM_CREATE:
		vga_render = VGARenderBlank;
		VGA_lines = 25;

        fp=fopen("bios\\video\\chr8x16.set","rb");
		if (fp) {
			fread(buffer,256*16,1,fp);
			fclose(fp);
		}
		else {
			MessageBox(hwnd,"ERROR: Unable to load VGA ROM character set. Text will not be visible on your screen","",MB_OK);
		}

        fp=fopen("bios\\video\\stdvga.pal","rb");
		if (fp) {
			for (x=0;x < 256;x++) {
				fread(&b1,1,1,fp); VGAPalette[x*3 + 0] = (int)b1;
				fread(&b1,1,1,fp); VGAPalette[x*3 + 1] = (int)b1;
				fread(&b1,1,1,fp); VGAPalette[x*3 + 2] = (int)b1;
			}
			fclose(fp);
		}
		else {
			MessageBox(hwnd,"ERROR: Unable to load standard VGA palette file STDVGA.PAL","",MB_OK);
		}

		for (x=0;x < 256;x++) {
			BitRev[x]  = (x&1)<<7;
			BitRev[x] |= (x&2)<<5;
			BitRev[x] |= (x&4)<<3;
			BitRev[x] |= (x&8)<<1;
			BitRev[x] |= (x&16)>>1;
			BitRev[x] |= (x&32)>>3;
			BitRev[x] |= (x&64)>>5;
			BitRev[x] |= (x&128)>>7;
		}

		VGA_pan_x=0;
		VGA_pan_y=0;

/* convert character map to "hardware" buffer

NOTE: I found a long time ago during the development of a program
      called FONTGDI that in reality many VGA cards support as many
	  as 32 scan lines per character cell. It's just that for some
	  reason many BIOSes prefer to hang if the calling program
	  specifies more than 16, which is probably why nobody knew about
	  this....

*/
		for (x=0,y=0;x < (256*16);) {
			for (w=0;w < 16;w++) VGACharacterRAM[y++]=buffer[x++];
			for (w=0;w < 16;w++) VGACharacterRAM[y++]=0;
		}

		VGAPaletteLog = (LOGPALETTE*)(malloc(sizeof(LOGPALETTE)+1200));
		VGAPaletteLog->palNumEntries=256;
		VGAPaletteLog->palVersion=0x300;
		for (x=0;x < 256;x++) {
			VGAPaletteLog->palPalEntry[x].peBlue=VGAPalette[x*3 + 2];
			VGAPaletteLog->palPalEntry[x].peGreen=VGAPalette[x*3 + 1];
			VGAPaletteLog->palPalEntry[x].peRed=VGAPalette[x*3 + 0];
			VGAPaletteLog->palPalEntry[x].peFlags=0;
		}
		VGARAMSize=512*1024;
		VGARAM = malloc(VGARAMSize+512);
		VGAScreenX = 320;
		VGAScreenY = 200;
		VGA_screenwidth = 640;
		VGA_screenheight = 400;
		VGARenderScaleX = (65536 * 640) / VGAScreenX;
		VGARenderScaleY = (65536 * 400) / VGAScreenY;
		VGAPaletteObj=CreatePalette(VGAPaletteLog);
		VGA_cursblinkdelay=0;
		if (!VGAPaletteObj) {
			MessageBox(NULL,"ERROR: UNABLE TO INITIALIZE PALETTE","",MB_OK);
			free(VGAPaletteLog);
			return -1;
		}

		VGADC = GetDC(hwnd);
		MoveWindow(hwnd,GetPrivateProfileInt("VGAWin","X",140,inifile),GetPrivateProfileInt("VGAWin","Y",0,inifile),GetPrivateProfileInt("VGAWin","W",GetSystemMetrics(SM_CXSCREEN)-150,inifile),GetPrivateProfileInt("VGAWin","H",
			GetSystemMetrics(SM_CYSCREEN)-120,inifile),TRUE);

		SetTimer(hwnd,2,40,NULL);

		VGARenderBuffer = malloc(640*405*3);
		VGAParams = (BITMAPINFO*)malloc(sizeof(BITMAPINFO)+1200);
		VGAParams->bmiHeader.biBitCount=24;
		VGAParams->bmiHeader.biClrImportant=256;
		VGAParams->bmiHeader.biClrUsed=256;
		VGAParams->bmiHeader.biCompression=0;
		VGAParams->bmiHeader.biHeight=-200;
		VGAParams->bmiHeader.biPlanes=1;
		VGAParams->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		VGAParams->bmiHeader.biSizeImage=320*200*3;
		VGAParams->bmiHeader.biWidth=320;
		for (x=0;x < 128;x++) {
			VGAParams->bmiColors[x].rgbBlue = (x*2);
			VGAParams->bmiColors[x].rgbGreen = 0;
			VGAParams->bmiColors[x].rgbRed = (x*2)+1;
			VGAParams->bmiColors[x].rgbReserved = 0;
		}

		VGARenderByRealDisplay(0,0,640,480);

		break;

#ifdef WIN95
	case WM_SIZING:
		rct = (LPRECT)lParam;

		w = VGAScreenX + (GetSystemMetrics(SM_CXEDGE)*2) + 4;
		h = VGAScreenY + (GetSystemMetrics(SM_CYEDGE)*2) + GetSystemMetrics(SM_CYCAPTION) + 4;

		if (wParam == WMSZ_TOP)
			rct->top = rct->bottom - h;
		if (wParam == WMSZ_BOTTOM)
			rct->bottom = rct->top + h;
		if (wParam == WMSZ_LEFT)
			rct->left = rct->right - w;
		if (wParam == WMSZ_RIGHT)
			rct->right = rct->left + w;

		w = 640 + (GetSystemMetrics(SM_CXEDGE)*2) + 4;
		h = 400 + (GetSystemMetrics(SM_CYEDGE)*2) + GetSystemMetrics(SM_CYCAPTION) + 4;
		rw = abs(rct->right - rct->left) - ((GetSystemMetrics(SM_CXEDGE)*2) + 1);
		rh = abs(rct->bottom - rct->top) - ((GetSystemMetrics(SM_CYEDGE)*2) + GetSystemMetrics(SM_CYCAPTION) + 4);

		if (((int)(((double)rw) / VGA_AspectRatio)) > rh) {
			rh = (int)(((double)rw) / VGA_AspectRatio);
		}
		else {
			rw = (int)(((double)rh) * VGA_AspectRatio);
		}
		if (rw > 640) rw=640;
		if (rh > 400) rh=400;
		rw += (GetSystemMetrics(SM_CXEDGE)*2) + 4;
		rh += (GetSystemMetrics(SM_CYEDGE)*2) + GetSystemMetrics(SM_CYCAPTION) + 4;

		if (wParam == WMSZ_BOTTOMLEFT) {
			rct->bottom = (rct->top+rh);
			rct->left = (rct->right-rw);
		}
		if (wParam == WMSZ_BOTTOMRIGHT) {
			rct->bottom = (rct->top+rh);
			rct->right = (rct->left+rw);
		}
		if (wParam == WMSZ_TOPLEFT) {
			rct->top = (rct->bottom-rh);
			rct->left = (rct->right-rw);
		}
		if (wParam == WMSZ_TOPRIGHT) {
			rct->top = (rct->bottom-rh);
			rct->right = (rct->left+rw);
		}
		break;
#endif WIN95

	case WM_ACTIVATE:
		x = LOWORD(wParam);
		if (x == WA_ACTIVE || WA_CLICKACTIVE) {
			SelectPalette(VGADC,VGAPaletteObj,FALSE);
			RealizePalette(VGADC);
		}
		if (x == WA_INACTIVE) {
		}
		break;

	case WM_SIZE:
		VGAScreenX = LOWORD(lParam);
		VGAScreenY = HIWORD(lParam);
		if (VGAScreenX > 0 && VGAScreenY) {
			VGARedraw(0,0,VGAScreenX,VGAScreenY);
		}
		break;

	case WM_CHAR:
		PostMessage(hwndMainKeyb,WM_CHAR,wParam,lParam);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_KEYDOWN:
		PostMessage(hwndMainKeyb,WM_KEYDOWN,wParam,lParam);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_KEYUP:
		PostMessage(hwndMainKeyb,WM_KEYUP,wParam,lParam);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_LBUTTONUP:
		break;

	case WM_PAINT:
		SelectPalette(VGADC,VGAPaletteObj,FALSE);
		RealizePalette(VGADC);
		ValidateRect(hwnd,NULL);
		VGARedraw(0,0,VGAScreenX,VGAScreenY);
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_CLOSE:
// don't close, just hide yourself
		SendMessage(hwndMain,WM_COMMAND,IDM_WINDOWS_DISPLAY,0);
		break;
	
	case WM_TIMER:
		t = (int)wParam;

		if (t == 2) {
			for (y=0;y < VGA_lines;y++) {
				if (VGA_UpdateLine[y]) {
					y2=y;
					while (VGA_UpdateLine[y2] && y2 < VGA_lines) {
						VGA_UpdateLine[y2]=0;
						y2++;
					}

					VGA_Cursor_visdraw=0;
					VGARenderByVGA(0,y*VGA_TextFontHeight,640,y2*VGA_TextFontHeight + VGA_TextFontHeight);
					VGA_Cursor_visdraw=1;
					y=y2;
				}
			}

			VGA_cursblinkdelay++;
			if (VGA_cursblinkdelay >= 2) {
				h = VGA_Cursor_visblink;
				VGA_Cursor_visblink = !VGA_Cursor_visblink;
				VGA_cursblinkdelay=0;
			}

			VGA_Cursor_visdraw=0;

			if (VGA_Cursor_X != VGA_Cursor_PX || VGA_Cursor_Y != VGA_Cursor_PY)
				VGARenderByVGA(VGA_Cursor_PX*8,VGA_Cursor_PY*VGA_TextFontHeight,VGA_Cursor_PX*8 + 8,VGA_Cursor_PY*VGA_TextFontHeight + VGA_TextFontHeight);

			VGA_Cursor_visdraw=1;
			VGARenderByVGA(VGA_Cursor_X*8,VGA_Cursor_Y*VGA_TextFontHeight,VGA_Cursor_X*8 + 8,VGA_Cursor_Y*VGA_TextFontHeight + VGA_TextFontHeight);
			VGA_Cursor_visdraw=0;

			VGA_Cursor_PX = VGA_Cursor_X;
			VGA_Cursor_PY = VGA_Cursor_Y;
		}
		break;

	case WM_SYSCOMMAND:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);

	case WM_DESTROY:
		GetWindowRect(hwnd,&hwndRec);
		wsprintf(mmtbuf2,"%d",hwndRec.left); WritePrivateProfileString("VGAWin","X",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.top); WritePrivateProfileString("VGAWin","Y",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.right-hwndRec.left); WritePrivateProfileString("VGAWin","W",mmtbuf2,inifile);
		wsprintf(mmtbuf2,"%d",hwndRec.bottom-hwndRec.top); WritePrivateProfileString("VGAWin","H",mmtbuf2,inifile);
		DeleteObject(VGAPaletteObj);
		free(VGAPaletteLog);
		free(VGAParams);
		free(VGARenderBuffer);
		free(VGARAM);
		VGAPaletteLog=NULL;
		VGAParams=NULL;
		VGARenderBuffer=NULL;
		VGARAM=NULL;

		ReleaseDC(hwnd,VGADC);
		break;

	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW)));
		break;

default:
		return DefWindowProc(hwnd,iMessage,wParam,lParam);
	}
	return 0L;
}

void VGA_SetMode(int x)
{
	int blank = !(x&0x80);

	VGA_mode = x&0x7F;

	if (VGA_mode == 0x13) {
		vga_render = VGARender320x200x256;
		VGA_OffsetBank = 0xA0000;
		VGA_lines = 200;
		VGA_TextFontHeight = 2;
		if (blank) memset(VGARAM,0,320*200);
	}
	else {
		vga_render = VGARender8Walpha;
		VGA_OffsetBank = 0xB8000;
		VGA_lines = 25;
		VGA_TextFontHeight = 16;
		if (blank) memset(VGARAM,0,25*80*2);
	}

	VGARenderByVGA(0,0,VGA_screenwidth,VGA_screenheight);
}

// VGA I/O port emulation of CRTC registers 3D4h and 3D5h
// CRTC index register
DWORD vga_crtc_3D4h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = VGA_CRTCIndx;
	}

	if (dir == IO_PORT_OUT) {
// simulate ability to out to CRTC in one go using 16-bit out.
// a popular feature used by many MS-DOS demos and drivers.
		VGA_CRTCIndx = ((int)data)&0xFF;
		if (size >= 2) vga_crtc_3D5h(1,IO_PORT_OUT,data>>8);
	}

	return r;
}

// CRTC data register
DWORD vga_crtc_3D5h(int size,int dir,DWORD data)
{
	DWORD r;
	int x,y,yh;
	DWORD o,io;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)(VGA_CRTCREGS[VGA_CRTCIndx]&0xFF);
	}

	if (dir == IO_PORT_OUT) {
// update the cursor if necessary
		if (VGA_CRTCIndx == 0x0E || VGA_CRTCIndx == 0x0F) {
			int addr;

			if (VGA_CRTCIndx == 0x0F)		addr = (data&0xFF);
			else							addr = VGA_CRTCREGS[0x0F];

			if (VGA_CRTCIndx == 0x0E)		addr |= ((data&0xFF)<<8);
			else							addr |= (VGA_CRTCREGS[0x0E]<<8);

			VGA_Cursor_X = addr % 80;
			VGA_Cursor_Y = addr / 80;
		}
// specialty "kludge" register" for video mode sets
		if (VGA_CRTCIndx == 0x88) {
			VGA_SetMode(data&0xFF);
		}
// specialty "kludge" register" for scrolling up the screen quickly
		if (VGA_CRTCIndx == 0x89) {
			yh = 25 - (data&0xFF); if (yh < 0) yh = 0;
			o=0;
			io=(25 - yh)*160;
			y=0;

			while (yh > 0) {
				yh--;
				y++;

				for (x=0;x < 80;x++) {
					VGARAM[o++] = VGARAM[io++];
					VGARAM[o++] = VGARAM[io++];
				}
			}

			while (y < 25) {
				y++;

				for (x=0;x < 80;x++) {
					VGARAM[o++] = 0x20;
					VGARAM[o++] = 0x07;
				}
			}

			VGARenderByVGA(0,0,VGA_screenwidth,VGA_screenheight);
		}

		VGA_CRTCREGS[VGA_CRTCIndx]=(BYTE)(data&0xFF);
	}

	return r;
}

// VGA palette index register
DWORD vga_pal_3C8h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = VGA_Palregidx/3;
	}

	if (dir == IO_PORT_OUT) {
		VGA_Palregidx = (((int)data)&0xFF)*3;
	}

	return r;
}

// VGA palette data register
DWORD vga_pal_3C9h(int size,int dir,DWORD data)
{
	DWORD r;
	int x;

	r=0;

	if (dir == IO_PORT_IN) {
		r = ((DWORD)(VGAPalette[VGA_Palregidx]&0xFF))>>2;
		VGA_Palregidx++;
		if (VGA_Palregidx >= 768) VGA_Palregidx=0;
	}

	if (dir == IO_PORT_OUT) {
		VGAPalette[VGA_Palregidx] = (BYTE)((data&0x3F)<<2);
		VGA_Palregidx++;
		if (VGA_Palregidx >= 768) VGA_Palregidx=0;
		for (x=0;x < VGA_lines;x++) VGA_UpdateLine[x]=1;;
	}

	return r;
}

// VGA video RAM emulation
DWORD vga_mem_RAM(int size,int mod,DWORD dat,DWORD addr)
{
	DWORD r;
	int banko,x,y;

	if (!VGA_powered) return 0xFFFFFFFF;

	r=0;
	banko = addr - VGA_OffsetBank;
	if (banko < 0 || banko > 0xFFFF) return 0xFFFFFFFF;
	if (size > 4) size=4;
	if (!size) return 0;

	if (mod == AB_MOD_READ || mod == AB_MOD_SNAPSHOT) {
		addr += size-1;

		for (x=0;x < size;x++) {
			r = (r<<8) | ((DWORD)VGARAM[banko]);

			if (!banko) {
				banko=0xFFFF;
			}
			else {
				banko--;
			}
		}
	}

	if (mod == AB_MOD_WRITE || mod == AB_MOD_SNAPSHOTWRITE) {
		for (x=0;x < size;x++) {
			VGARAM[banko++] = (BYTE)(dat&0xFF);
			dat >>= 8;

			if (banko & 0x10000) {
				banko=0;
			}
		}
		banko -= size;

		if (banko >= 0 && banko <= 0xFFFF) {
// update the display
			if (VGA_mode == 0x13) {
				x = banko%320;
				y = banko/320;
				if (y < 200) {
					if (code_freerun) {
						VGA_UpdateLine[y]=1;
					}
					else {
						VGA_Cursor_visdraw=0;
						VGARenderByVGA(x*2,y*2,x*2 + 2,y*2 + 2);
						VGA_Cursor_visdraw=1;
					}
				}
			}
			else {
				x = (banko>>1)%80;
				y = (banko>>1)/80;
				if ((y*VGA_TextFontHeight) < 400) {
					if (code_freerun) {
						VGA_UpdateLine[y]=1;
					}
					else {
						VGA_Cursor_visdraw=0;
						VGARenderByVGA(x*8,y*VGA_TextFontHeight,x*8 + 8,y*VGA_TextFontHeight + VGA_TextFontHeight);
						VGA_Cursor_visdraw=1;
					}
				}
			}
		}
	}

	return r;
}

// VGA reset code
void VGAReset()
{
	DWORD x;
	int a,b;
	char *teststr = "If you can see this, VGA emulation was successfully initialized";

	if (!VGA_powered) return;

	assign_IO_port(0x3C8,vga_pal_3C8h);
	assign_IO_port(0x3C9,vga_pal_3C9h);
	assign_IO_port(0x3D4,vga_crtc_3D4h);
	assign_IO_port(0x3D5,vga_crtc_3D5h);

	for (x=0xA0000;x < 0xC0000;x += 0x1000) addrbus_make_page_adaptor(x,vga_mem_RAM);
	VGA_SetMode(0);

// make VGA "test pattern"
	x=0;
	for (b=0;b < 16;b++) {
		for (a=0;a < 64;a += 4) {
			VGARAM[x++] = (BYTE)' ';
			VGARAM[x++] = (BYTE)(((a>>2)&0xF) | ((b&0xF)<<4));
			VGARAM[x++] = (BYTE)177;
			VGARAM[x++] = (BYTE)(((a>>2)&0xF) | ((b&0xF)<<4));
			VGARAM[x++] = (BYTE)178;
			VGARAM[x++] = (BYTE)(((a>>2)&0xF) | ((b&0xF)<<4));
			VGARAM[x++] = (BYTE)219;
			VGARAM[x++] = (BYTE)(((a>>2)&0xF) | ((b&0xF)<<4));
		}

		for (;a < 80;a += 2) {
			VGARAM[x++] = (BYTE)((b&1) ? 250 : 254);
			VGARAM[x++] = (BYTE)(a-64);
			VGARAM[x++] = (BYTE)((b&1) ? 254 : 250);
			VGARAM[x++] = (BYTE)(a-63);
		}
	}

	for (;b < 17;b++) {
		for (a=0;a < 80 && teststr[a];a++) {
			VGARAM[x++] = (BYTE)teststr[a];
			VGARAM[x++] = (BYTE)0x1E;
		}
		for (;a < 80;a++) {
			VGARAM[x++] = (BYTE)' ';
			VGARAM[x++] = (BYTE)0x1E;
		}
	}

	for (;b < 18;b++) {
		for (a=0;a < 80;a++) {
			VGARAM[x++] = (BYTE)194;
			VGARAM[x++] = (BYTE)0x0F;
		}
	}

	for (;b < 25;b++) {
		for (a=0;a < 80;a++) {
			VGARAM[x++] = (BYTE)197;
			VGARAM[x++] = (BYTE)0x0F;
		}
	}

	VGARenderByVGA(0,0,VGAScreenX,VGAScreenY);
}

void VGAPowerOn()
{
	vga_render = VGARender8Walpha;

	VGA_powered=1;
	VGAReset();
}

void VGAPowerOff()
{
	DWORD x;

	vga_render = VGARenderBlank;
	VGARenderByVGA(0,0,VGAScreenX,VGAScreenY);

	VGA_powered=0;
	unassign_IO_port(0x3C8);
	unassign_IO_port(0x3C9);
	unassign_IO_port(0x3D4);
	unassign_IO_port(0x3D5);
	for (x=0xA0000;x < 0xC0000;x += 0x1000) addrbus_make_page_empty(x);
}

void VGAInit()
{
	vga_render = VGARenderBlank;
	VGA_powered=0;
}

void VGAClose()
{
	VGAPowerOff();
}

int VGAIsPowered()
{
	return VGA_powered;
}
