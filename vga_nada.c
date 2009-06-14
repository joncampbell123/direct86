/*

rendering function for VGA emulation in stockvga.c
Renders blank VGA display. Used when virtual VGA "device" is powered off
or display is disabled.

*/

#include "global.h"
#include "stockvga.h"

extern int							VGA_screenwidth,VGA_screenheight;

void VGARenderBlank(int x1,int y1,int x2,int y2)
{
	int x,y;
	BYTE *VGAptr;

	if (!VGAParams || !VGARenderBuffer) return;

	if (x1 > x2 || y1 > y2) return;

	for (y=y1;y <= y2;y++) {
		VGAptr = VGARenderBuffer + (((VGA_screenwidth*y)+x1)*3);
		for (x=x1;x <= x2;x++) {
// We must optimize this part of the loop greatly
			*VGAptr++=0; *VGAptr++=0; *VGAptr++=0;
		}
	}
}
