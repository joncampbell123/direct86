/*

rendering function for VGA emulation in stockvga.c
Renders alphanumeric VGA display mode with 8xn alpha-numeric characters.

Relies on character ram array in STOCKVGA.C

*/

#include "global.h"
#include "stockvga.h"

extern int							VGA_screenwidth,VGA_screenheight;

void VGARender8Walpha(int x1,int y1,int x2,int y2)
{
	int x,y,i,xo;
	BYTE backrgb[3],forergb[3];
	BYTE *rgbptr[2] = {&forergb[0],&backrgb[0]};
	BYTE *rgbrptr;
	BYTE *r1,*r2;
	BYTE *VGAptr;
	BYTE *VGAramptr;
	BYTE Char,FGColor,BGColor;
	int textrow,textrowp,textcol,textcolp;
	int vga_cursor_draw,vga_cursor_doit;

	if (!VGAParams || !VGARenderBuffer) return;

	vga_cursor_draw = VGA_Cursor_visible && VGA_Cursor_visblink && VGA_Cursor_visdraw;
	x1 &= ~7;
	x2 = (x2 + 7) & ~7; if (x2 >= VGA_screenwidth) x2 = VGA_screenwidth-1;

	if (x1 >= x2 || y1 >= y2) return;

	textrow    = y1 / VGA_TextFontHeight;
	textrowp   = y1 % VGA_TextFontHeight;
	for (y=y1;y <= y2;y++) {
		VGAptr = VGARenderBuffer + (((VGA_screenwidth*y)+x1)*3);
		textcol =x1>>3;
		textcolp=0;
		vga_cursor_doit = vga_cursor_draw && textrow == VGA_Cursor_Y && textrowp >= (VGA_TextFontHeight-3) && textrowp <= (VGA_TextFontHeight-2);
		VGAramptr = VGARAM + (textrow * 160) + (textcol*2);
		for (x=x1;x <= x2;x += 8) {
// We must optimize this part of the loop greatly
			Char = *VGAramptr++;
			FGColor = *VGAramptr++;
			BGColor = FGColor >> 4;
			FGColor &= 0xF;

			r1 = (&backrgb[0])+2; r2 = VGAPalette + ((FGColor<<1) + FGColor);
			*r1-- = *r2++;
			*r1-- = *r2++;
			*r1-- = *r2++;

			r1 = (&forergb[0])+2; r2 = VGAPalette + ((BGColor<<1) + BGColor);
			*r1-- = *r2++;
			*r1-- = *r2++;
			*r1-- = *r2++;

// make "patch overs" for hardware cursor. much of the decision here
// was pre-decided to reduce IFs during the loop so things go faster
			if (vga_cursor_doit) {
				if (textcol == VGA_Cursor_X)	i=0xFF;
				else							i=BitRev[VGACharacterRAM[(Char<<5)+textrowp]];
			}
			else {
				i=BitRev[VGACharacterRAM[(Char<<5)+textrowp]];
			}

// are we at an "on" pixel?
			for (xo=0;xo < 8;xo++) {
				rgbrptr = rgbptr[i&1];
				*VGAptr++=*rgbrptr++;
				*VGAptr++=*rgbrptr++;
				*VGAptr++=*rgbrptr++;
				i >>= 1;
			}

			textcol++;
		}
		textrowp++;
		if (textrowp >= VGA_TextFontHeight) {
			textrowp=0;
			textrow++;
		}
	}
}
