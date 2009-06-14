/*

rendering function for VGA emulation in stockvga.c
Renders alphanumeric VGA display mode with 8xn alpha-numeric characters.

Relies on character ram array in STOCKVGA.C

*/

#include "global.h"
#include "stockvga.h"

extern int							VGA_screenwidth,VGA_screenheight;

void VGARender320x200x256(int x1,int y1,int x2,int y2)
{
	int x,y;
	BYTE *palptr;
	BYTE *VGAptr;
	BYTE *VGAramptr;
	int cidx;
	int textrow,textrowp,textcol,textcolp;
	int vga_cursor_draw;

	if (!VGAParams || !VGARenderBuffer) return;

	vga_cursor_draw = VGA_Cursor_visible && VGA_Cursor_visblink && VGA_Cursor_visdraw;
	x1 &= ~1;
	x2 = (x2 + 1) & ~1; if (x2 >= VGA_screenwidth) x2 = VGA_screenwidth-1;

	if (x1 >= x2 || y1 >= y2) return;

	textrow    = y1>>1;
	textrowp   = y1&1;
	for (y=y1;y <= y2;y++) {
		VGAptr = VGARenderBuffer + (((VGA_screenwidth*y)+x1)*3);
		textcol =x1>>1;
		textcolp=0;
		VGAramptr = VGARAM + (textrow * 320) + textcol;
		for (x=x1;x <= x2;x += 2) {
// We must optimize this part of the loop greatly
			cidx = ((int)*VGAramptr++);
			palptr = VGAPalette + cidx + (cidx<<1) + 2;

// are we at an "on" pixel?
			*VGAptr++=*palptr--;
			*VGAptr++=*palptr--;
			*VGAptr++=*palptr; palptr += 2;
			*VGAptr++=*palptr--;
			*VGAptr++=*palptr--;
			*VGAptr++=*palptr;

			textcol++;
		}
		textrowp++;
		if (textrowp & 2) {
			textrowp=0;
			textrow++;
		}
	}
}
