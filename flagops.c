
#include "global.h"
#include "cpuexec.h"
#include "flagops.h"

// set the various flags based upon a value of x
void setflagsuponval(void *x,int l)
{
	BYTE b;
	WORD w;
	DWORD dw;
	BYTE parity;
	int bit,bad;
	BYTE *ptr;

	if (l == 1) {
// 8-28-2k apparently the "RESERVED" bit is set if either Z or S
// is set, at least that's how my Pentium I 166MHz does it
		b = *((BYTE*)x);
		if (b & 0x80)
			ireg_flags |= 0x80;
		else
			ireg_flags &= ~0x80;
		if (!b)
			ireg_flags |= 0x40;
		else
			ireg_flags &= ~0x40;
	}
	if (l == 2) {
		w = *((WORD*)x);
		if (w & 0x8000)
			ireg_flags |= 0x80;
		else
			ireg_flags &= ~0x80;
		if (!w)
			ireg_flags |= 0x40;
		else
			ireg_flags &= ~0x40;
	}
	if (l == 4) {
		dw = *((DWORD*)x);
		if (dw & 0x80000000)
			ireg_flags |= 0x80;
		else
			ireg_flags &= ~0x80;
		if (!dw)
			ireg_flags |= 0x40;
		else
			ireg_flags &= ~0x40;
	}

// now for the hardest part: emulating the PE (parity) flag.
// apparently what you do is add the bits together, take the
// lowest bit, XOR it by 1, and there it is, the parity value
	ptr = (BYTE*)x;
	parity = 1;
	for (bit=1;bit;bit--) {
		bad = (int)*ptr++;
		parity += bad; bad >>= 1;
		parity += bad; bad >>= 1;
		parity += bad; bad >>= 1;
		parity += bad; bad >>= 1;
		parity += bad; bad >>= 1;
		parity += bad; bad >>= 1;
		parity += bad; bad >>= 1;
		parity += bad;
	}

	if (parity&1)		ireg_flags |= 4;
	else				ireg_flags &= ~4;
}

void setflagsuponval_mask(void *x,int l,int mask)
{
	int ac;

	ac = ireg_flags & mask;
	setflagsuponval(x,l);
	ireg_flags &= ~mask;
	ireg_flags |= ac;
}
