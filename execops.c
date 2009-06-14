
#include "global.h"
#include "cpuexec.h"
#include "cpustkw.h"
#include "stackops.h"
#include "flagops.h"
#include "brkpts.h"

extern int					do_not_log_sp;
extern DWORD				exec_begin_cs,exec_begin_eip;

void exec_MOV(void *s,void *d,int l)
{
	if (l == 1)
		*((BYTE*)d) = *((BYTE*)s);
	if (l == 2)
		*((WORD*)d) = *((WORD*)s);
	if (l == 4)
		*((DWORD*)d) = *((DWORD*)s);
}

void exec_MOVZX(void *s,void *d,int ol,int nl)
{
	DWORD ov;

	if (ol == 1)
		ov = (DWORD)*((BYTE*)s);
	if (ol == 2)
		ov = (DWORD)*((WORD*)s);
	if (ol == 4)
		ov = (DWORD)*((DWORD*)s);

	if (nl == 2)
		*((WORD*)d) = (WORD)ov;
	if (nl == 4)
		*((DWORD*)d) = ov;
}

void exec_BSF386(void *s,void *d,int l)
{
	WORD r,dd;

	if (l == 2) {
		dd = *((WORD*)s);
// 8-22-99: Apprently if the first operand is zero, BSF ignores it
// entirely and does not change the register.
		ireg_flags &= ~0xC1;		// carry, sign flags are always cleared by BSF
		ireg_flags |= 0x14;			// parity and aux are always set

		if (dd) {
			ireg_flags &= ~0x40;	// in these cases the zero flag is clear
			ireg_flags |= 0x10;
			if (dd & 0xF) ireg_flags |= 0x10;
			for (r=0;r < 16 && !(dd & (1<<r));r++);
			*((WORD*)d) = r;
		}
		else {
			ireg_flags |= 0x40;	// in these cases the zero flag is clear
			ireg_flags &= ~0x10;
		}
	}
}

void exec_BTR386(void *s,void *d,int l,int bit)
{
	if (l == 1) {
		BYTE r,dd;

		r = dd = *((BYTE*)s);

		l &= 7;
		r &= ~(1 << bit);
		if (dd != r)	ireg_flags |= 1;
		else			ireg_flags &= ~1;
		ireg_flags &= 0x800;		// OF is always cleared

		*((BYTE*)d) = r;
	}
	if (l == 2) {
		WORD r,dd;

		r = dd = *((WORD*)s);

		l &= 15;
		r &= ~(1 << bit);
		if (dd != r)	ireg_flags |= 1;
		else			ireg_flags &= ~1;
		ireg_flags &= 0x800;		// OF is always cleared

		*((WORD*)d) = r;
	}
}

void exec_LDS(void *s,void *d,int l)
{
	if (l == 2) {
		*((WORD*)d) = *((WORD*)s) & 0xFFFF;
// don't forget DS
		ireg_ds = *(((WORD*)s)+1) & 0xFFFF;
	}
}

void exec_LES(void *s,void *d,int l)
{
	if (l == 2) {
		*((WORD*)d) = *((WORD*)s) & 0xFFFF;
// don't forget ES
		ireg_es = *(((WORD*)s)+1) & 0xFFFF;
	}
}

void exec_LSS(void *s,void *d,int l)
{
	if (l == 2) {
		*((WORD*)d) = *((WORD*)s) & 0xFFFF;
// don't forget SS
		ireg_ss = *(((WORD*)s)+1) & 0xFFFF;
	}
}

// in DEC and INC, the Auxillary Carry flag is
// set if the operation overflows (i.e. FFFF becomes
// 0000 or vice versa). If not, it is cleared.
// 8-22-99: Apparently Carry is not touched
// 8-29-2k: Apparently Aux is set of lower nibble overflows (Fh => 0h)
// 8-29-2k: Apparently transition of 7FFFh to 8000h sets Overflow flag

void exec_INC(void *s,void *d,int l)
{
	if (l == 1) {
		if (((++(*((BYTE*)s)))&0xF) == 0)		ireg_flags |= 0x10;
		else									ireg_flags &= ~0x10;

		if (*((BYTE*)s) == 0x80)				ireg_flags |= 0x800;
		else									ireg_flags &= ~0x800;
	}
	if (l == 2) {
		if (((++(*((WORD*)s)))&0xF) == 0)		ireg_flags |= 0x10;
		else									ireg_flags &= ~0x10;

		if (*((WORD*)s) == 0x8000)				ireg_flags |= 0x800;
		else									ireg_flags &= ~0x800;
	}
	if (l == 4) {
		if (((++(*((DWORD*)s)))&0xF) == 0)		ireg_flags |= 0x10;
		else									ireg_flags &= ~0x10;

		ireg_flags &= ~0x800;
	}

	setflagsuponval(s,l);
}

void exec_DEC(void *s,void *d,int l)
{
	if (l == 1) {
		if (((--(*((BYTE*)s)))&0xF) == 0xF)		ireg_flags |= 0x10;
		else									ireg_flags &= ~0x10;

		ireg_flags &= ~0x800;
	}
	if (l == 2) {
		if (((--(*((WORD*)s)))&0xF) == 0xF)			ireg_flags |= 0x10;
		else										ireg_flags &= ~0x10;

		ireg_flags &= ~0x800;
	}
	if (l == 4) {
		if (((--(*((DWORD*)s)))&0xF) == 0xF)		ireg_flags |= 0x10;
		else										ireg_flags &= ~0x10;

		ireg_flags &= ~0x800;
	}

	setflagsuponval(s,l);
}

void exec_AND(void *s,void *d,int l)
{
	if (l == 1)			*((BYTE*)d) &= *((BYTE*)s);
	if (l == 2)			*((WORD*)d) &= *((WORD*)s);
	if (l == 4)			*((DWORD*)d) &= *((DWORD*)s);

	ireg_flags &= ~0x811;
	setflagsuponval(d,l);
}

void exec_SBB(void *s,void *d,int l)
{
	BOOL carry,ove;
	DWORD pd=0;
	WORD pw=0;
	BYTE pb=0;

	carry = ireg_flags & 1;
	if (l == 1) {
		pb = *((BYTE*)d);
		*((BYTE*)d) -= *((BYTE*)s);

		if (carry) {
			(*((BYTE*)d))--;

			setflagsuponval(d,l);

			if (*((BYTE*)s) == 0xFF) {
				ove=0;
				carry=1;
			}
			else {
				if ((*((BYTE*)d)) > pb)					carry=1;
				else									carry=0;

				if (carry && ((*((BYTE*)s)) & 0x80))	ove=1;
				else									ove=0;
			}
		}
		else {
			setflagsuponval(d,l);

			if ((*((BYTE*)d)) > pb)					carry=1;
			else									carry=0;

			if (carry && !((*((BYTE*)s)) & 0x80))	ove=1;
			else									ove=0;
		}

// emulation of weird undocumented "feature" 8-22-99:
// for some reason (I'm not sure if it's true of every CPU but seems
// to happen with my Pentium 166) the processor likes to set AUX to
// 1 if the following calculation is true:
//
// given operation a + b = c, (b AND 0Fh) < (a AND 0Fh)
// so, emulate it!
		ireg_flags &= ~0x11;
		ireg_flags |= carry;
		if (((*((BYTE*)s)) & 0xF) < (pb & 0xF))
			ireg_flags |= 0x10;

		ireg_flags &= ~0x800;
		ireg_flags |= (ove * 0x800);

		return;
	}
	if (l == 2) {
		pw = *((WORD*)d);
		*((WORD*)d) -= *((WORD*)s);

		if (carry) {
			(*((WORD*)d))--;

			setflagsuponval(d,l);

			if (*((WORD*)s) == 0xFFFF) {
				ove=0;
				carry=1;
			}
			else {
				if ((*((WORD*)d)) > pw)					carry=1;
				else									carry=0;

				if (carry && ((*((WORD*)s)) & 0x8000) && !(pw & 0x8000))	ove=1;
				else														ove=0;
			}
		}
		else {
			setflagsuponval(d,l);

			if ((*((WORD*)d)) > pw)					carry=1;
			else									carry=0;

			if (carry && !((*((WORD*)s)) & 0x8000))	ove=1;
			else									ove=0;
		}
	}
	if (l == 4) {
		pd = *((DWORD*)d);
		*((DWORD*)d) -= *((DWORD*)s);
		if (carry) (*((DWORD*)d))--;
	}

	setflagsuponval(d,l);
// check for a carry and overflow condition
	if (l == 1) {
	}
	if (l == 2) {
	}
	if (l == 4) {
		if ((*((DWORD*)d)) > pd || ((*((DWORD*)s)) == 0xFFFFFFFF && carry))	carry=1;
		else														carry=0;

		if (carry && ((*((DWORD*)s)) & 0x80000000))
			ove=1;
		else
			ove=0;
	}
// emulation of weird undocumented "feature" 8-22-99:
// for some reason (I'm not sure if it's true of every CPU but seems
// to happen with my Pentium 166) the processor likes to set AUX to
// 1 if the following calculation is true:
//
// given operation a + b = c, (b AND 0Fh) < (a AND 0Fh)
// so, emulate it!
	ireg_flags &= ~0x11;
	ireg_flags |= carry;
	if (l == 1) {
		if (((*((BYTE*)s)) & 0xF) > (pb & 0xF))
			ireg_flags |= 0x10;
	}
	if (l == 2) {
		if (((*((WORD*)s)) & 0xF) > (pw & 0xF))
			ireg_flags |= 0x10;
	}
	if (l == 4) {
		if (((*((DWORD*)s)) & 0xF) > (pd & 0xF))
			ireg_flags |= 0x10;
	}
	ireg_flags &= ~0x800;
	ireg_flags |= (ove * 0x800);
}

void exec_SUB(void *s,void *d,int l)
{
	BOOL carry,ove;
	DWORD pd;
	WORD pw;
	BYTE pb;

	if (l == 1) {
		pb = *((BYTE*)d);
		*((BYTE*)d) -= *((BYTE*)s);
	}
	if (l == 2) {
		pw = *((WORD*)d);
		*((WORD*)d) -= *((WORD*)s);
	}
	if (l == 4) {
		pd = *((DWORD*)d);
		*((DWORD*)d) -= *((DWORD*)s);
	}

	setflagsuponval(d,l);
// check for a carry and overflow condition
	if (l == 1) {
		if ((*((BYTE*)d)) > pb)		carry=1;
		else						carry=0;

// the overflow flag is designed for handling signed integers

// special case #1
		if (!(*((BYTE*)d))) {
			ove=0;
		}
// special case #2
		if ((*((BYTE*)s)) == 0x00) {
			ove=0;
		}
// special case #3
		else if ((*((BYTE*)s)) == 0x80 && pb == 0) {
			ove=1;
		}
// special case #4
		else if ((*((BYTE*)s)) == 0x80 && pb == 0x80) {
			ove=0;
		}
// special case #5
		else if ((*((BYTE*)s)) == 0xFF && pb == 0x7F) {
			ove=1;
		}
// special case #6
		else if ((*((BYTE*)s)) == 0x80 && !(pb & 0x80)) {
			ove=1;
		}
		else if ((pb & 0x80) && !((*((BYTE*)d)) & 0x80) && !((*((BYTE*)s)) & 0x80)) {
			ove=1;
		}
		else {
			ove=0;
		}
	}
	if (l == 2) {
		if ((*((WORD*)d)) > pw)		carry=1;
		else						carry=0;

// special case #1
		if (!(*((WORD*)d))) {
			ove=0;
		}
// special case #2
		else if ((*((WORD*)s)) == 0x00) {
			ove=0;
		}
// special case #3
		else if ((*((WORD*)s)) == 0x8000 && pw == 0) {
			ove=1;
		}
// special case #4
		else if ((*((WORD*)s)) == 0x8000 && pw == 0x8000) {
			ove=0;
		}
// special case #5
		else if ((*((WORD*)s)) == 0x8000 && !(pw & 0x8000)) {
			ove=1;
		}
// special case #6
		else if (((*((WORD*)s)) & 0x8000) && ((*((WORD*)d)) & 0x8000) && !(pw & 0x8000)) {
			ove=1;
		}
		else if ((pw & 0x8000) && !((*((WORD*)d)) & 0x8000) && !((*((WORD*)s)) & 0x8000)) {
			ove=1;
		}
		else {
			ove=0;
		}
	}
	if (l == 4) {
		if ((*((DWORD*)d)) > pd)	carry=1;
		else						carry=0;

// special case #1
		if (!(*((DWORD*)d))) {
			ove=0;
		}
// special case #2
		else if ((*((DWORD*)s)) == 0x00) {
			ove=0;
		}
// special case #3
		else if ((*((DWORD*)s)) == 0x80000000 && pd == 0) {
			ove=1;
		}
// special case #4
		else if ((*((DWORD*)s)) == 0x80000000 && pd == 0x80000000) {
			ove=0;
		}
// special case #5
		else if ((*((DWORD*)s)) == 0x80000000 && !(pd & 0x80000000)) {
			ove=1;
		}
// special case #6
		else if (((*((DWORD*)s)) & 0x80000000) && ((*((DWORD*)d)) & 0x80000000) && !(pd & 0x80000000)) {
			ove=1;
		}
		else if ((pd & 0x80000000) && !((*((DWORD*)d)) & 0x80000000) && !((*((DWORD*)s)) & 0x80000000)) {
			ove=1;
		}
		else {
			ove=0;
		}
	}
// emulation of weird undocumented "feature" 8-22-99:
// for some reason (I'm not sure if it's true of every CPU but seems
// to happen with my Pentium 166) the processor likes to set AUX to
// 1 if the following calculation is true:
//
// given operation a + b = c, (b AND 0Fh) < (a AND 0Fh) sets aux
// so, emulate it!
	ireg_flags &= ~0x11;
	ireg_flags |= carry;
	if (l == 1) {
		if (((*((BYTE*)s)) & 0xF) > (pb & 0xF))
			ireg_flags |= 0x10;
	}
	if (l == 2) {
		if (((*((WORD*)s)) & 0xF) > (pw & 0xF))
			ireg_flags |= 0x10;
	}
	if (l == 4) {
		if (((*((DWORD*)s)) & 0xF) > (pd & 0xF))
			ireg_flags |= 0x10;
	}
	ireg_flags &= ~0x800;
	ireg_flags |= (ove ? 0x800 : 0);
}

void exec_CMP(void *s,void *d,int l)
{
	DWORD dw,dw2;
	WORD w,w2;
	BYTE b,b2;

/* according to Intel's documentation, CMP works
by subtracting dest - src (in some internal register,
I guess), and updating the flags based upon the
difference. we do the same here */
	if (l == 1) {
		b = *((BYTE*)s);
		b2 = *((BYTE*)d);
		exec_SUB(&b,&b2,1);
	}
	if (l == 2) {
		w = *((WORD*)s);
		w2 = *((WORD*)d);
		exec_SUB(&w,&w2,2);
	}
	if (l == 4) {
		dw = *((DWORD*)s);
		dw2 = *((DWORD*)d);
		exec_SUB(&dw,&dw2,4);
	}
}

void exec_ADC(void *s,void *d,int l)
{
	DWORD pd;
	WORD pw;
	BYTE pb;
	BOOL carry,ove;

	carry = ireg_flags&1;

	if (l == 1) {
		pb = *((BYTE*)d);
		*((BYTE*)d) += (BYTE)(*((BYTE*)s) + carry);
	}
	if (l == 2) {
		pw = *((WORD*)d);
		*((WORD*)d) += (WORD)(*((WORD*)s) + carry);
	}
	if (l == 4) {
		pd = *((DWORD*)d);
		*((DWORD*)d) += (DWORD)(*((DWORD*)s) + carry);
	}

	setflagsuponval(d,l);
// check for a carry and overflow condition
	if (l == 1) {
		if ((*((BYTE*)d)) < pb || ((*((BYTE*)s)) == 0xFF && carry))	carry=1;
		else														carry=0;

		if (carry && ((*((BYTE*)s)) & 0x80))
			ove=1;
		else
			ove=0;
	}
	if (l == 2) {
		if ((*((WORD*)d)) < pw || ((*((WORD*)s)) == 0xFFFF && carry))	carry=1;
		else														carry=0;

		if (carry && ((*((WORD*)s)) & 0x8000))
			ove=1;
		else
			ove=0;
	}
	if (l == 4) {
		if ((*((DWORD*)d)) < pd || ((*((DWORD*)s)) == 0xFFFFFFFF && carry))	carry=1;
		else														carry=0;

		if (carry && ((*((DWORD*)s)) & 0x80000000))
			ove=1;
		else
			ove=0;
	}
// emulation of weird undocumented "feature" 8-22-99:
// for some reason (I'm not sure if it's true of every CPU but seems
// to happen with my Pentium 166) the processor likes to set AUX to
// 1 if the following calculation is true:
//
// given operation a + b = c, (b AND 0Fh) > (0Fh - (a AND 0Fh))
// so, emulate it!
	ireg_flags &= ~0x11;
	ireg_flags |= carry;
	if (l == 1) {
		if (((*((BYTE*)s)) & 0xF) > (0xF - (pb & 0xF)))
			ireg_flags |= 0x10;
	}
	if (l == 2) {
		if (((*((WORD*)s)) & 0xF) > (0xF - (pw & 0xF)))
			ireg_flags |= 0x10;
	}
	if (l == 4) {
		if (((*((DWORD*)s)) & 0xF) > (0xF - (pd & 0xF)))
			ireg_flags |= 0x10;
	}
	ireg_flags &= ~0x800;
	ireg_flags |= (ove * 0x800);
}

void exec_ADD(void *s,void *d,int l)
{
	DWORD pd;
	WORD pw;
	BYTE pb;
	BOOL carry,ove;

	if (l == 1) {
		pb = *((BYTE*)d);
		*((BYTE*)d) += *((BYTE*)s);
	}
	if (l == 2) {
		pw = *((WORD*)d);
		*((WORD*)d) += *((WORD*)s);
	}
	if (l == 4) {
		pd = *((DWORD*)d);
		*((DWORD*)d) += *((DWORD*)s);
	}

	setflagsuponval(d,l);
// check for a carry and overflow condition
	if (l == 1) {
		if ((*((BYTE*)d)) < pb)		carry=1;
		else						carry=0;

		if (carry && ((*((BYTE*)s)) & 0x80))
			ove=1;
		else
			ove=0;
	}
	if (l == 2) {
		if ((*((WORD*)d)) < pw)		carry=1;
		else						carry=0;

		if (carry && ((*((WORD*)s)) & 0x8000))
			ove=1;
		else
			ove=0;
	}
	if (l == 4) {
		if ((*((DWORD*)d)) < pd)	carry=1;
		else						carry=0;

		if (carry && ((*((DWORD*)s)) & 0x80000000))
			ove=1;
		else
			ove=0;
	}
// emulation of weird undocumented "feature" 8-22-99:
// for some reason (I'm not sure if it's true of every CPU but seems
// to happen with my Pentium 166) the processor likes to set AUX to
// 1 if the following calculation is true:
//
// given operation a + b = c, (b AND 0Fh) > (0Fh - (a AND 0Fh))
// so, emulate it!
	ireg_flags &= ~0x11;
	ireg_flags |= carry;
	if (l == 1) {
		if (((*((BYTE*)s)) & 0xF) > (0xF - (pb & 0xF)))
			ireg_flags |= 0x10;
	}
	if (l == 2) {
		if (((*((WORD*)s)) & 0xF) > (0xF - (pw & 0xF)))
			ireg_flags |= 0x10;
	}
	if (l == 4) {
		if (((*((DWORD*)s)) & 0xF) > (0xF - (pd & 0xF)))
			ireg_flags |= 0x10;
	}
	ireg_flags &= ~0x800;
	ireg_flags |= (ove * 0x800);
}

void exec_CALL(void *s,void *d,int l)
{
	if (l == 2) {
		do_not_log_sp=1;
		NearCall16(*((WORD*)s));
		AddStackWatchEntry(SWT_CALLNEAR,ireg_cs,ireg_eip,ireg_cs,exec_begin_eip,ireg_ss,ireg_ss,ireg_esp,ireg_esp-2);
		do_not_log_sp=0;
	}
}

void exec_FARCALL(void *s,void *d,int l)
{
	DWORD oip,ocs;

	if (l == 2) {
		do_not_log_sp=1;
		oip = ireg_eip;
		ocs = ireg_cs;
		FarCall16(*(((WORD*)s)+1),*((WORD*)s));
		AddStackWatchEntry(SWT_CALLFAR,ireg_cs,ireg_eip,exec_begin_cs,exec_begin_eip,ireg_ss,ireg_ss,ireg_esp,ireg_esp-4);
		do_not_log_sp=0;
	}
}

void exec_JMP(void *s,void *d,int l)
{
	if (l == 2)
		ireg_eip = (DWORD)(*((WORD*)s));
}

void exec_FARJMP(void *s,void *d,int l)
{
	if (l == 2) {
		ireg_cs = (DWORD)(*(((WORD*)s)+1));
		ireg_eip = (DWORD)(*((WORD*)s));
	}
}

void exec_PUSH(void *s,void *d,int l)
{
	PushWord(*((WORD*)s));
}

void exec_POP(void *s,void *d,int l)
{
	*((WORD*)s) = (WORD)PopWord();
}

void exec_TEST(void *s,void *d,int l)
{
	BYTE b;
	WORD w;
	DWORD dw;

	if (l == 1) {
		b = *((BYTE*)s);
		b &= *((BYTE*)d);
		setflagsuponval(&b,1);
	}
	if (l == 2) {
		w = *((WORD*)s);
		w &= *((WORD*)d);
		setflagsuponval(&w,2);
	}
	if (l == 4) {
		dw = *((DWORD*)s);
		dw &= *((DWORD*)d);
		setflagsuponval(&dw,4);
	}
	ireg_flags &= ~0x811;			// apparently CARRY and AUX CARRY are cleared
}

void exec_NOT(void *s,void *d,int l)
{
	if (l == 1)
		*((BYTE*)s) ^= 0xFF;
	if (l == 2)
		*((WORD*)s) ^= 0xFFFF;
	if (l == 4)
		*((DWORD*)s) ^= 0xFFFFFFFF;

// 5/9/99 - I discovered that in the case of the NOT instruction,
// the flags are not affected
//	setflagsuponval_mask(s,l,0x40);
}

void exec_NEG(void *s,void *d,int l)
{
	char buf[8];

	ireg_flags &= ~0x811;			// apparently CARRY and AUX CARRY are cleared
// 8-22-99: Apprently a lot of weird operations go on in the real thing
// because the flag settings turn out weird in my testings. Oh well...
// Weird thing 1: Carry flag is set if the value is non-zero
// Weird thing 2: Aux Carry flag is set if the value (bits 0-14) is non-zero
// Weird thing 3: Overflow flag is set if the value is the largest possible negative integer (i.e. 0x80)
	if (l == 1) {
		ireg_flags |= ((*((BYTE*)s)) ? 1 : 0);
		ireg_flags |= (((*((BYTE*)s)) & 0x7F) ? 0x10 : 0);
		ireg_flags |= (((*((BYTE*)s)) == 0x80) ? 0x800 : 0);
	}
	if (l == 2) {
		ireg_flags |= ((*((WORD*)s)) ? 1 : 0);
		ireg_flags |= (((*((WORD*)s)) & 0x7FFF) ? 0x10 : 0);
		ireg_flags |= (((*((WORD*)s)) == 0x8000) ? 0x800 : 0);
	}
	if (l == 4) {
		ireg_flags |= ((*((DWORD*)s)) ? 1 : 0);
		ireg_flags |= (((*((DWORD*)s)) & 0x7FFFFFFF) ? 0x10 : 0);
		ireg_flags |= (((*((DWORD*)s)) == 0x80000000) ? 0x800 : 0);
	}

	if (l == 1)
		*((BYTE*)s) = ((*((BYTE*)s)) ^ 0xFF)+1;
	if (l == 2)
		*((WORD*)s) = ((*((WORD*)s)) ^ 0xFFFF)+1;
	if (l == 4)
		*((DWORD*)s) = ((*((DWORD*)s)) ^ 0xFFFFFFFF)+1;

// 9-5-99: Apparently the parity value is calculated with the most significant bit ignored
	memcpy(buf,s,l);
	if (l == 1)		*((BYTE*)s) &= 0x7F;
	if (l == 2)		*((WORD*)s) &= 0x7FFF;
	if (l == 4)		*((DWORD*)s) &= 0x7FFFFFFF;
	setflagsuponval(s,l);
	memcpy(s,buf,l);

// 2-17-2000: The zero flag is only set when the result is zero
	if (l == 1) {
		if (*((BYTE*)s)) ireg_flags &= ~0x40;
	}
	if (l == 2) {
		if (*((WORD*)s)) ireg_flags &= ~0x40;
	}
	if (l == 4) {
		if (*((DWORD*)s)) ireg_flags &= ~0x40;
	}
}

void exec_MUL(void *s,void *d,int l)
{
	unsigned int v,v2;

// 8-22-98: If an overflow occurs, both CARRY and OVERFLOW are set
	if (l == 1) {
		v = ((int)(ireg_eax&255)) * ((int)(*((BYTE*)s)));
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= ((WORD)(v&0xFFFF));
		if (v & 0xFFFFFF00)
			ireg_flags |= 0x801;
		else
			ireg_flags &= ~0x801;
	}
	if (l == 2) {
		v = ((int)(ireg_eax&0xFFFF)) * ((int)(*((WORD*)s)));
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= ((DWORD)(v&0xFFFF));
		ireg_edx &= 0xFFFF0000;
		ireg_edx |= ((DWORD)((v>>16)&0xFFFF));
		if (v & 0xFFFF0000)
			ireg_flags |= 0x801;
		else
			ireg_flags &= ~0x801;
	}
	if (l == 4) {
		v = ireg_eax;
		v2 = *((DWORD*)s);
		__asm {
			mov		eax,v
			mul		v2
			mov		ireg_eax,eax
			mov		ireg_edx,edx
		}
		ireg_flags &= ~0x801;

		if (ireg_edx) ireg_flags |= 0x801;
	}
}

void exec_IMUL(void *s,void *d,int l)
{
	int v,v2;

// 8-22-98: If an overflow occurs, both CARRY and OVERFLOW are set
	if (l == 1) {
		v = ((int)((char)(ireg_eax&255))) * ((int)(*((char*)s)));
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= ((WORD)(v&0xFFFF));
		if (v >= 0x80 || v < 0x80)	v=1;
		else						v=0;

		ireg_flags &= ~0x801;
		if (v) ireg_flags |= 0x801;
	}
	if (l == 2) {
		v = ((int)(ireg_eax&0xFFFF)) * ((int)(*((signed short int*)s)));
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= ((WORD)(v&0xFFFF));
		ireg_edx &= 0xFFFF0000;
		ireg_edx |= ((WORD)((v>>16)&0xFFFF));
		if (v >= 0x8000 || v < 0x8000)	v=1;
		else							v=0;

		ireg_flags &= ~0x801;

		if (v) ireg_flags |= 0x801;
	}
	if (l == 4) {
		v = ireg_eax;
		v2 = *((int*)s);
		__asm {
			mov		eax,v
			imul	v2
			mov		ireg_eax,eax
			mov		ireg_edx,edx
		}
// IMUL overflow conditions are a little more complex to test...
		v=0;
		if (ireg_edx & 0x80000000) {
			if (ireg_edx > 0 && !(ireg_edx == 1 && ireg_eax == 0))
				v=1;
		}
		else {
			if (ireg_edx > 0)
				v=1;
		}

		ireg_flags &= ~0x801;

		if (v) ireg_flags |= 0x801;
	}
}

void exec_IMUL386(void *s,void *d,int l)
{
	int v;

	if (l == 2) {
		v = ((int)(*((signed short int*)d))) * ((int)(*((signed short int*)s)));
		*((short int*)d) = (short int)v;
		if (v <= -32769 || v >= 32768)
			ireg_flags |= 0x800;
		else
			ireg_flags &= ~0x800;
	}
}

void exec_DIV(void *s,void *d,int l)
{
// the DIV instruction apparently handles the flags in the wierdest way
// (probably related to the way the division is done internally):
//
// given x / y = z,
//
// ZF = set if x == y
// SF = (1-((x/y)&1))
// PF = parity value of (x-y)
// CF = AF = (1-(((x/y)>>1)&1))
// OF = 0
	unsigned int v,v2,x;
	unsigned long lx,lv,lv2;

	if (l == 1) {
		v = ((unsigned int)(ireg_eax&0xFFFF));
		v2 = ((unsigned int)(*((BYTE*)s)));
		if (!v2) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		x = ((unsigned)v) / ((unsigned)v2);
		if (x & 0xFFFFFF00) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= x;
		x = ((unsigned)v) % ((unsigned)v2);
		ireg_eax |= x<<8;

		x = v-v2;
		setflagsuponval(&x,1);
		ireg_flags &= ~0x40;
		if (v == v2) ireg_flags |= 0x40;
		ireg_flags &= ~0x80;
		if (1-((v/v2)&1)) ireg_flags |= 0x80;
		ireg_flags &= ~0x11;
		if (1-(((v/v2)>>1)&1)) ireg_flags |= 0x11;
		ireg_flags &= ~0x800;
	}
	if (l == 2) {
		v = ((unsigned int)(ireg_eax&0xFFFF));
		v |= (((unsigned int)(ireg_edx&0xFFFF)))<<16;
		v2 = ((unsigned int)(*((WORD*)s)));
		if (!v2) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		x = ((unsigned int)v) / ((unsigned int)v2);
		if (x & 0xFFFF0000) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= x;
		x = ((unsigned)v) % ((unsigned)v2);
		ireg_edx &= 0xFFFF0000;
		ireg_edx |= x;

		x = v-v2;
		setflagsuponval(&x,2);
		ireg_flags &= ~0x40;
		if (v == v2) ireg_flags |= 0x40;
		ireg_flags &= ~0x80;
		if (1-((v/v2)&1)) ireg_flags |= 0x80;
		ireg_flags &= ~0x11;
		if (1-(((v/v2)>>1)&1)) ireg_flags |= 0x11;
		ireg_flags &= ~0x800;
	}
	if (l == 4) {
		lv = (unsigned long)ireg_eax;
		lv |= (unsigned long)(ireg_edx<<32);
		lv2 = (unsigned long)(*((unsigned int*)s));
		if (!lv2) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		lx = ((unsigned long)lv) / ((unsigned long)lv2);
		if (lx & 0xFFFFFFFF00000000) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		ireg_eax = (DWORD)lx;
		lx = ((unsigned long)lv) % ((unsigned long)lv2);
		ireg_edx = (DWORD)lx;

		x = (int)(lv-lv2);
		setflagsuponval(&x,4);
		ireg_flags &= ~0x40;
		if (lv == lv2) ireg_flags |= 0x40;
		ireg_flags &= ~0x80;
		if (1-((lv/lv2)&1)) ireg_flags |= 0x80;
		ireg_flags &= ~0x11;
		if (1-(((lv/lv2)>>1)&1)) ireg_flags |= 0x11;
		ireg_flags &= ~0x800;
	}
}

void exec_IDIV(void *s,void *d,int l)
{
	int v,v2,x;
	long lx,lv,lv2;

	if (l == 1) {
		v = ((int)(ireg_eax&0xFFFF));
		v2 = ((int)(*((char*)s)));
		if (!v2) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		x = v / v2;
		if (x & 0x7FFFFF00) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= (WORD)x;
		x = v % v2;
		ireg_eax |= x<<8;
	}
	if (l == 2) {
		v = ((int)(ireg_eax&0xFFFF));
		v |= (((int)(ireg_edx&0xFFFF)))<<16;
		v2 = ((int)(*((short int*)s)));
		if (!v2) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		x = v / v2;
		if (x & 0x7FFF0000) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		ireg_eax &= 0xFFFF0000;
		ireg_eax |= x;
		x = v % v2;
		ireg_edx &= 0xFFFF0000;
		ireg_edx |= x;
	}
	if (l == 4) {
		lv = ireg_eax;
		lv |= (ireg_edx<<32);
		lv2 = (long)(*((int*)s));
		if (!lv2) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		lx = lv / lv2;
		if (lx & 0x7FFFFFFF00000000) {
			SignalInterrupt16(0);		// division overflow!
			CheckTriggerSWINTBreakpoint(0);
			return;
		}
		ireg_eax = (DWORD)lx;
		lx = lv % lv2;
		ireg_edx = (DWORD)lx;
	}
}

void exec_XOR(void *s,void *d,int l)
{
	if (l == 1)
		*((BYTE*)d) ^= *((BYTE*)s);
	if (l == 2)
		*((WORD*)d) ^= *((WORD*)s);
	if (l == 4)
		*((DWORD*)d) ^= *((DWORD*)s);

	setflagsuponval(d,l);
	ireg_flags &= ~0x811;			// apparently CARRY and AUX CARRY are cleared
}

void exec_OR(void *s,void *d,int l)
{
	if (l == 1)
		*((BYTE*)d) |= *((BYTE*)s);
	if (l == 2)
		*((WORD*)d) |= *((WORD*)s);
	if (l == 4)
		*((DWORD*)d) |= *((DWORD*)s);

	pf_8088=1;						// 6/25/2k - Apparently PF made to act like 8088
	setflagsuponval(d,l);
	pf_8088=0;
	ireg_flags &= ~0x811;			// apparently CARRY and AUX CARRY are cleared
}

void exec_ROR(void *s,void *d,int l)
{
	BOOL ov;

	if (!exec_ROR_o) ireg_flags &= ~0x800;

	if (l == 1) {
		ireg_flags = (ireg_flags & ~1) | (*((BYTE*)s)&1);
		*((BYTE*)s) = (*((BYTE*)s) >> 1) | ((*((BYTE*)s)<<7)&0x80);
		ov = *((BYTE*)s) & 0x40;
	}
	if (l == 2) {
		ireg_flags = (ireg_flags & ~1) | (*((WORD*)s)&1);
		*((WORD*)s) = (*((WORD*)s) >> 1) | ((*((WORD*)s)<<15)&0x8000);
		ov = *((WORD*)s) & 0x4000;
	}
	if (l == 4) {
		ireg_flags = (ireg_flags & ~1) | (*((DWORD*)s)&1);
		*((DWORD*)s) = (*((DWORD*)s) >> 1) | ((*((DWORD*)s)<<31)&0x80000000);
		ov = *((DWORD*)s) & 0x40000000;
	}
	if (ireg_flags & 1 || ov) ireg_flags |= 0x800;

	setflagsuponval(s,l);
	ireg_flags &= ~(0x90 | 0x40);
}

void exec_ROR_CL(void *s,void *d,int l)
{
	int cx;

	cx = ireg_ecx&0x1F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	exec_ROR_o=FALSE;

	while (cx--) {
		exec_ROR(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

void exec_ROR_imm(void *s,void *d,int l)
{
	int cx;

	cx = exec_imm1 & 0x1F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	exec_ROR_o=FALSE;

	while (cx--) {
		exec_ROR(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

void exec_ROL(void *s,void *d,int l)
{
	BOOL ov;

	if (!exec_ROR_o) ireg_flags &= ~0x800;

	if (l == 1) {
		ireg_flags = (ireg_flags & ~1) | (*((BYTE*)s)>>7);
		*((BYTE*)s) = (*((BYTE*)s) << 1) | (*((BYTE*)s)>>7);
		ov = *((BYTE*)s) & 0x40;
	}
	if (l == 2) {
		ireg_flags = (ireg_flags & ~1) | (*((WORD*)s)>>15);
		*((WORD*)s) = (*((WORD*)s) << 1) | (*((WORD*)s)>>15);
		ov = *((WORD*)s) & 0x4000;
	}
	if (l == 4) {
		ireg_flags = (ireg_flags & ~1) | (*((DWORD*)s)>>31);
		*((DWORD*)s) = (*((DWORD*)s) << 1) | (*((DWORD*)s)>>31);
		ov = *((DWORD*)s) & 0x40000000;
	}

	if (ireg_flags & 1 || ov) ireg_flags |= 0x800;

	setflagsuponval(s,l);
	ireg_flags &= ~0x90;
}

void exec_ROL_CL(void *s,void *d,int l)
{
	int cx;

	cx = ireg_ecx&0x1F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	exec_ROR_o=FALSE;

	while (cx--) {
		exec_ROL(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

void exec_ROL_imm(void *s,void *d,int l)
{
	int cx;

	cx = exec_imm1 & 0x1F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	exec_ROR_o=FALSE;

	while (cx--) {
		exec_ROL(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

void exec_RCL(void *s,void *d,int l)
{
	int c,ov;

// 8-23-99: Until now my impression was that this instruction took
// the carry bit and put it in the least significant bit of the
// result. Not so, it seems what was in Carry before this instruction
// was executed is totally irrelevant and doesn't mean a thing.
// However, during multiple shifts it does hold the shifted out bit
// in an internal register and empty THAT into the LSB on the next
// shift. Overflow is set if the bit in the internal register was
// shifted out (multiple shifts only). The Zero flag is NOT set if
// the bit in the internal register is set, even if the lower 8/16/32
// bits are all 0.
	if (exec_ROR_o)
		c = exec_RC_carry;
	else
		c = 0;

	ireg_flags &= ~0x800;

	if (l == 1) {
		ireg_flags = (ireg_flags & ~1) | (*((BYTE*)s)>>7);
		exec_RC_carry = ireg_flags & 1;

		*((BYTE*)s) = (*((BYTE*)s) << 1) | c;
		ov = (*((BYTE*)s)) & 0xC0;
	}
	if (l == 2) {
		ireg_flags = (ireg_flags & ~1) | (*((WORD*)s)>>15);
		exec_RC_carry = ireg_flags & 1;

		*((WORD*)s) = (*((WORD*)s) << 1) | c;
		ov = (*((WORD*)s)) & 0xC000;
	}
	if (l == 4) {
		ireg_flags = (ireg_flags & ~1) | (*((DWORD*)s)>>31);
		exec_RC_carry = ireg_flags & 1;

		*((DWORD*)s) = (*((DWORD*)s) << 1) | c;
		ov = (*((DWORD*)s)) & 0xC0000000;
	}

	if (ov)		ireg_flags |= 0x800;
	else		ireg_flags &= ~0x800;

	setflagsuponval(s,l);
	if (exec_RC_carry) ireg_flags &= ~0x40;
	ireg_flags &= ~0x90;
}

void exec_RCL_CL(void *s,void *d,int l)
{
	int cx;

	exec_ROR_o=FALSE;
	exec_RC_carry=0;

	cx = ireg_ecx&0x001F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	while (cx--) {
		exec_RCL(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

void exec_RCL_imm(void *s,void *d,int l)
{
	int cx;

	exec_ROR_o=FALSE;
	exec_RC_carry=0;

	cx = exec_imm1 & 0x001F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	while (cx--) {
		exec_RCL(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

void exec_RCR(void *s,void *d,int l)
{
	int c,ov;

// 8-23-99: Until now my impression was that this instruction took
// the carry bit and put it in the most significant bit of the
// result. Not so, it seems what was in Carry before this instruction
// was executed is totally irrelevant and doesn't mean a thing.
// However, during multiple shifts it does hold the shifted out bit
// in an internal register and empty THAT into the MSB on the next
// shift. Overflow is set if the bit in the internal register was
// shifted out (multiple shifts only). The Zero flag is NOT set if
// the bit in the internal register is set, even if the lower 8/16/32
// bits are all 0.
	if (exec_ROR_o)
		c = exec_RC_carry;
	else
		c = 0;

	if (l == 1) {
		ireg_flags = (ireg_flags & ~1) | (*((BYTE*)s)&1);
		exec_RC_carry = ireg_flags & 1;

		*((BYTE*)s) = (*((BYTE*)s) >> 1) | (c<<7);
		ov = (*((BYTE*)s)) & 0xC0;
	}
	if (l == 2) {
		ireg_flags = (ireg_flags & ~1) | (*((WORD*)s)&1);
		exec_RC_carry = ireg_flags & 1;

		*((WORD*)s) = (*((WORD*)s) >> 1) | (c<<15);
		ov = (*((WORD*)s)) & 0xC000;
	}
	if (l == 4) {
		ireg_flags = (ireg_flags & ~1) | (*((DWORD*)s)&1);
		exec_RC_carry = ireg_flags & 1;

		*((DWORD*)s) = (*((DWORD*)s) >> 1) | (c<<31);
		ov = (*((DWORD*)s)) & 0xC0000000;
	}

	if (ov)		ireg_flags |= 0x800;
	else		ireg_flags &= ~0x800;

	setflagsuponval(s,l);
	if (exec_RC_carry) ireg_flags &= ~0x40;
	ireg_flags &= ~0x90;
}

void exec_RCR_CL(void *s,void *d,int l)
{
	int cx;

	exec_ROR_o=FALSE;
	exec_RC_carry=0;

	cx = ireg_ecx&0x1F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	while (cx--) {
		exec_RCR(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

void exec_RCR_imm(void *s,void *d,int l)
{
	int cx;

	exec_ROR_o=FALSE;
	exec_RC_carry=0;

	cx = exec_imm1 & 0x1F;
	if (!cx) {
		ireg_flags &= ~0x891;
		ireg_flags |= 0x40;
		return;
	}

	while (cx--) {
		exec_RCR(s,d,l);
		exec_ROR_o=TRUE;
	}

	exec_ROR_o=FALSE;
}

// apparently the SHR, SHL, etc, instructions set carry if anything
// shifted out
void exec_SHL(void *s,void *d,int l)
{
	BOOL carry;

	if (l == 1) {
		carry = (*((BYTE*)s) & 0x80)?1:0;
		*((BYTE*)s) <<= 1;
	}
	if (l == 2) {
		carry = (*((WORD*)s) & 0x8000)?1:0;
		*((WORD*)s) <<= 1;
	}
	if (l == 4) {
		carry = (*((DWORD*)s) & 0x80000000)?1:0;
		*((DWORD*)s) <<= 1;
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0
// 9-6-99: apparently overflow is if carry is set or the sign bit is set
	ireg_flags &= ~0x800;

	if (l == 1) {
		if ((*((BYTE*)s) & 0x80) || carry) ireg_flags |= 0x800;
	}
	if (l == 2) {
		if ((*((WORD*)s) & 0x8000) || carry) ireg_flags |= 0x800;
	}
	if (l == 4) {
		if ((*((DWORD*)s) & 0x80000000) || carry) ireg_flags |= 0x800;
	}

	ireg_flags |= 0x10;
}

void exec_SHL_CL(void *s,void *d,int l)
{
	int cx;
	BOOL carry;

	ireg_flags &= 0x891;

	cx = ireg_ecx & 0x1F;
	if (!cx) {
		setflagsuponval(s,l);
// 8-22-99: apprently the zero flag is set if CX == 0
		ireg_flags &= 0x40;
		ireg_flags &= 0x891;		// and carry, aux, sign, and overflow are zero
		return;
	}
	carry=FALSE;

// 8-22-99: Contrary to earlier impression carry only reflects the
// last bit shifted out
	if (l == 1) {
		while (cx--) {
			carry = (*((BYTE*)s) & 0x80) ? 1 : 0;

			*((BYTE*)s) <<= 1;
		}
	}
	if (l == 2) {
		while (cx--) {
			carry = (*((WORD*)s) & 0x8000) ? 1 : 0;

			*((WORD*)s) <<= 1;
		}
	}
	if (l == 4) {
		while (cx--) {
			carry = (*((DWORD*)s) & 0x80000000) ? 1 : 0;

			*((DWORD*)s) <<= 1;
		}
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0
// 9-6-99: apparently overflow is set if all bits except the most significant are 0 after the shift AND carry is set
	ireg_flags &= ~0x800;

	if (l == 1) {
		if ((*((BYTE*)s) & 0x80) || carry) ireg_flags |= 0x800;
	}
	if (l == 2) {
		if ((*((WORD*)s) & 0x8000) || carry) ireg_flags |= 0x800;
	}
	if (l == 4) {
		if ((*((DWORD*)s) & 0x80000000) || carry) ireg_flags |= 0x800;
	}

	ireg_flags |= 0x10;
}

void exec_SHL_imm(void *s,void *d,int l)
{
	int cx;
	BOOL carry;

	ireg_flags &= 0x891;

	cx = exec_imm1 & 0x1F;
	if (!cx) {
		setflagsuponval(s,l);
// 8-22-99: apprently the zero flag is set if immediate shift value == 0
		ireg_flags &= 0x40;
		ireg_flags &= 0x891;		// and carry, aux, sign, and overflow are zero
		return;
	}
	carry=FALSE;

// 8-22-99: Contrary to earlier impression carry only reflects the
// last bit shifted out
	if (l == 1) {
		while (cx--) {
			carry = (*((BYTE*)s) & 0x80) ? 1 : 0;

			*((BYTE*)s) <<= 1;
		}
	}
	if (l == 2) {
		while (cx--) {
			carry = (*((WORD*)s) & 0x8000) ? 1 : 0;

			*((WORD*)s) <<= 1;
		}
	}
	if (l == 4) {
		while (cx--) {
			carry = (*((DWORD*)s) & 0x80000000) ? 1 : 0;

			*((DWORD*)s) <<= 1;
		}
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0
// 9-6-99: apparently overflow is set if all bits except the most significant are 0 after the shift AND carry is set
	ireg_flags &= ~0x800;

	if (l == 1) {
		if ((*((BYTE*)s) & 0x80) || carry) ireg_flags |= 0x800;
	}
	if (l == 2) {
		if ((*((WORD*)s) & 0x8000) || carry) ireg_flags |= 0x800;
	}
	if (l == 4) {
		if ((*((DWORD*)s) & 0x80000000) || carry) ireg_flags |= 0x800;
	}

	ireg_flags |= 0x10;
}

void exec_SHR(void *s,void *d,int l)
{
	BOOL carry;

	if (l == 1) {
		carry = (*((BYTE*)s) & 1)?1:0;
		*((BYTE*)s) >>= 1;
	}
	if (l == 2) {
		carry = (*((WORD*)s) & 1)?1:0;
		*((WORD*)s) >>= 1;
	}
	if (l == 4) {
		carry = (*((DWORD*)s) & 1)?1:0;
		*((DWORD*)s) >>= 1;
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0
// 9-6-99: apparently overflow is set if the 6th/14th/30th bit is set
	ireg_flags &= ~0x800;

	if (l == 1) {
		if ((*((BYTE*)s) & 0xC0) || carry) ireg_flags |= 0x800;
	}
	if (l == 2) {
		if ((*((WORD*)s) & 0xC000) || carry) ireg_flags |= 0x800;
	}
	if (l == 4) {
		if ((*((DWORD*)s) & 0xC0000000) || carry) ireg_flags |= 0x800;
	}

	ireg_flags |= 0x10;
}

void exec_SHR_CL(void *s,void *d,int l)
{
	int cx;
	BOOL carry;

	ireg_flags &= 0x891;

	cx = ireg_ecx & 0x1F;
	if (!cx) {
		setflagsuponval(s,l);
// 8-22-99: apprently the zero flag is set if CX == 0
		ireg_flags &= 0x40;
		ireg_flags &= 0x891;		// and carry, aux, sign, and overflow are zero
		return;
	}
	carry=FALSE;

// 8-22-99: Contrary to earlier impression carry only reflects the
// last bit shifted out
	if (l == 1) {
		while (cx--) {
			carry = (*((BYTE*)s) & 1);

			*((BYTE*)s) >>= 1;
		}
	}
	if (l == 2) {
		while (cx--) {
			carry = (*((WORD*)s) & 1);

			*((WORD*)s) >>= 1;
		}
	}
	if (l == 4) {
		while (cx--) {
			carry = (*((DWORD*)s) & 1);

			*((DWORD*)s) >>= 1;
		}
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0
// 9-6-99: apparently overflow is set if the 6th/14th/30th bit is set
	ireg_flags &= ~0x800;

	if (l == 1) {
		if ((*((BYTE*)s) & 0x40) || carry) ireg_flags |= 0x800;
	}
	if (l == 2) {
		if ((*((WORD*)s) & 0x4000) || carry) ireg_flags |= 0x800;
	}
	if (l == 4) {
		if ((*((DWORD*)s) & 0x40000000) || carry) ireg_flags |= 0x800;
	}

	ireg_flags |= 0x10;
}

void exec_SHR_imm(void *s,void *d,int l)
{
	int cx;
	BOOL carry;

	ireg_flags &= 0x891;

	cx = exec_imm1 & 0x1F;
	if (!cx) {
		setflagsuponval(s,l);
// 8-22-99: apprently the zero flag is set if CX == 0
		ireg_flags &= 0x40;
		ireg_flags &= 0x891;		// and carry, aux, sign, and overflow are zero
		return;
	}
	carry=FALSE;

// 8-22-99: Contrary to earlier impression carry only reflects the
// last bit shifted out
	if (l == 1) {
		while (cx--) {
			carry = (*((BYTE*)s) & 1);

			*((BYTE*)s) >>= 1;
		}
	}
	if (l == 2) {
		while (cx--) {
			carry = (*((WORD*)s) & 1);

			*((WORD*)s) >>= 1;
		}
	}
	if (l == 4) {
		while (cx--) {
			carry = (*((DWORD*)s) & 1);

			*((DWORD*)s) >>= 1;
		}
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0
// 9-6-99: apparently overflow is set if the 6th/14th/30th bit is set
	ireg_flags &= ~0x800;

	if (l == 1) {
		if ((*((BYTE*)s) & 0xC0) || carry) ireg_flags |= 0x800;
	}
	if (l == 2) {
		if ((*((WORD*)s) & 0xC000) || carry) ireg_flags |= 0x800;
	}
	if (l == 4) {
		if ((*((DWORD*)s) & 0xC0000000) || carry) ireg_flags |= 0x800;
	}

	ireg_flags |= 0x10;
}

void exec_SAR(void *s,void *d,int l)
{
	BOOL carry;

	if (l == 1) {
		carry = (*((BYTE*)s) & 1)?1:0;
		*((BYTE*)s) = ((*((BYTE*)s)) >> 1) | (*((BYTE*)s) & 0x80);
	}
	if (l == 2) {
		carry = (*((WORD*)s) & 1)?1:0;
		*((WORD*)s) = ((*((WORD*)s)) >> 1) | (*((WORD*)s) & 0x8000);
	}
	if (l == 4) {
		carry = (*((DWORD*)s) & 1)?1:0;
		*((DWORD*)s) = ((*((DWORD*)s)) >> 1) | (*((DWORD*)s) & 0x80000000);
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set, and overflow is always cleared
	ireg_flags &= ~0x800;
	ireg_flags |= 0x10;
}

void exec_SAR_CL(void *s,void *d,int l)
{
	int cx;
	BOOL carry;

	ireg_flags &= 0x891;

	cx = ireg_ecx & 0x1F;
	if (!cx) {
		setflagsuponval(s,l);
// 8-22-99: apprently the zero flag is set if CX == 0
		ireg_flags &= 0x40;
		ireg_flags &= 0x891;		// and carry, aux, sign, and overflow are zero
		return;
	}
	carry=FALSE;

// 8-22-99: Contrary to earlier impression carry only reflects the
// last bit shifted out
	if (l == 1) {
		BYTE v;

		v = *((BYTE*)s);
		while (cx--) {
			carry = (v & 1);

			v >>= 1;
		}
		*((BYTE*)s) = (*((BYTE*)s) & 0x80) | v;
	}
	if (l == 2) {
		WORD v;

		v = *((WORD*)s);
		while (cx--) {
			carry = (v & 1);

			v >>= 1;
		}
		*((WORD*)s) = (*((WORD*)s) & 0x8000) | v;
	}
	if (l == 4) {
		DWORD v;

		v = *((DWORD*)s);
		while (cx--) {
			carry = (v & 1);

			v >>= 1;
		}
		*((DWORD*)s) = (*((DWORD*)s) & 0x80000000) | v;
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0, and overflow is always cleared
	ireg_flags &= ~0x800;
	ireg_flags |= 0x10;
}

void exec_SAR_imm(void *s,void *d,int l)
{
	int cx;
	BOOL carry;

	ireg_flags &= 0x891;

	cx = exec_imm1 & 0x1F;
	if (!cx) {
		setflagsuponval(s,l);
// 8-22-99: apprently the zero flag is set if CX == 0
		ireg_flags &= 0x40;
		ireg_flags &= 0x891;		// and carry, aux, sign, and overflow are zero
		return;
	}
	carry=FALSE;

// 8-22-99: Contrary to earlier impression carry only reflects the
// last bit shifted out
	if (l == 1) {
		BYTE v;

		v = *((BYTE*)s);
		while (cx--) {
			carry = (v & 1);

			v >>= 1;
		}
		*((BYTE*)s) = (*((BYTE*)s) & 0x80) | v;
	}
	if (l == 2) {
		WORD v;

		v = *((WORD*)s);
		while (cx--) {
			carry = (v & 1);

			v >>= 1;
		}
		*((WORD*)s) = (*((WORD*)s) & 0x8000) | v;
	}
	if (l == 4) {
		DWORD v;

		v = *((DWORD*)s);
		while (cx--) {
			carry = (v & 1);

			v >>= 1;
		}
		*((DWORD*)s) = (*((DWORD*)s) & 0x80000000) | v;
	}

	setflagsuponval(s,l);
	if (carry)		ireg_flags |= 0x1;
	else			ireg_flags &= ~0x1;
// 8-22-99: apprently aux is always set if CX != 0, and overflow is always cleared
	ireg_flags &= ~0x800;
	ireg_flags |= 0x10;
}

void exec_XCHG(void *s,void *d,int l)
{
	BYTE b,b2;
	WORD w,w2;
	DWORD dw,dw2;

	if (l == 1) {
		b = *((BYTE*)s);
		b2 = *((BYTE*)d);
		*((BYTE*)s) = b2;
		*((BYTE*)d) = b;
	}
	if (l == 2) {
		w = *((WORD*)s);
		w2 = *((WORD*)d);
		*((WORD*)s) = w2;
		*((WORD*)d) = w;
	}
	if (l == 4) {
		dw = *((DWORD*)s);
		dw2 = *((DWORD*)d);
		*((DWORD*)s) = dw2;
		*((DWORD*)d) = dw;
	}

// 8-22-99: Apprently XCHG leaves flags untouched
}

void exec_XADD(void *s,void *d,int l)
{
	exec_XCHG(s,d,l);
	exec_ADD(s,d,l);
}

void exec_LIDT(void *s,void *d,int l)
{
	if (l == 6) {
		ireg_idtr = (LONG)(*(DWORD*)s);
		ireg_idtr |= (((LONG)(*(DWORD*)d))<<16)<<16;
		ireg_idtr &= 0xFFFFFFFFFFFF;
	}
}

void exec_SIDT(void *s,void *d,int l)
{
	if (l == 6) {
		(*(DWORD*)s) = (DWORD)ireg_idtr;
		(*(DWORD*)d) &= (DWORD)0xFFFF0000;
		(*(DWORD*)d) |= (DWORD)(((ireg_idtr>>16)>>16) & 0xFFFF);
	}
}

void exec_SLDT(void *s,void *d,int l)
{
	if (l == 6) {
		(*(WORD*)s) = (WORD)ireg_ldtr;
	}
}

void exec_LGDT(void *s,void *d,int l)
{
	if (l == 6) {
		ireg_gdtr = (LONG)(*(DWORD*)s);
		ireg_gdtr |= (((LONG)(*(DWORD*)d))<<16)<<16;
		ireg_gdtr &= 0xFFFFFFFFFFFF;
	}
}

void exec_SGDT(void *s,void *d,int l)
{
	if (l == 6) {
		(*(DWORD*)s) = (DWORD)ireg_gdtr;
		(*(DWORD*)d) &= (DWORD)0xFFFF0000;
		(*(DWORD*)d) |= (DWORD)(((ireg_gdtr>>16)>>16) & 0xFFFF);
	}
}

void exec_LMSW(void *s,void *d,int l)
{
	if (l == 2) {
		ireg_cr0 &= ~0xFFFF;
		ireg_cr0 |= *((WORD*)s);
	}
}

void exec_SMSW(void *s,void *d,int l)
{
	if (l == 2) {
		(*(WORD*)s) = (WORD)ireg_cr0;
	}
}

void exec_SHLD386(void *s,void *d,int l,BYTE shif)
{
	WORD ws,wd;
	DWORD dws,dwd;

	shif &= 0x1F;
	if (!shif) {
// 8-22-99: SHLD acts like SHL when this condition exists
		ireg_flags &= ~0x40;
		ireg_flags &= ~0x891;
		return;
	}

	ireg_flags &= ~0x800;		// Overflow is always cleared
	ireg_flags |= 0x10;			// Aux is always set

	if (l == 2) {
		ws = *((WORD*)s);
		wd = *((WORD*)d);

		while (shif--) {
			ireg_flags &= ~1;

			if (wd & 0x8000) ireg_flags |= 1;

			wd <<= 1;
			wd |= (ws>>15);
			ws <<= 1;
		}

		*((WORD*)d) = wd;

		setflagsuponval(d,l);
	}
	if (l == 4) {
		dws = *((DWORD*)s);
		dwd = *((DWORD*)d);

		while (shif--) {
			ireg_flags &= ~1;

			if (dwd & 0x80000000) ireg_flags |= 1;

			dwd <<= 1;
			dwd |= (dws>>31);
			dws <<= 1;
		}

		*((DWORD*)d) = dwd;

		setflagsuponval(d,l);
	}
}

void exec_SHRD386(void *s,void *d,int l,BYTE shif)
{
	WORD ws,wd;
	DWORD dws,dwd;

	shif &= 0x1F;
	if (!shif) {
// 8-22-99: SHRD acts like SHR when this condition exists
		ireg_flags &= ~0x40;
		ireg_flags &= ~0x891;
		return;
	}

	ireg_flags &= ~0x800;		// Overflow is always cleared
	ireg_flags |= 0x10;			// Aux is always set

	if (l == 2) {
		ws = *((WORD*)s);
		wd = *((WORD*)d);

		while (shif--) {
			ireg_flags &= ~1;

			if (wd & 1) ireg_flags |= 1;

			ws >>= 1;
			ws |= (wd&1)<<15;
			wd >>= 1;
		}

		*((WORD*)d) = wd;

		setflagsuponval(d,l);
	}
	if (l == 4) {
		dws = *((DWORD*)s);
		dwd = *((DWORD*)d);

		while (shif--) {
			ireg_flags &= ~1;

			if (dwd & 1) ireg_flags |= 1;

			dws >>= 1;
			dws |= (dwd&1)<<15;
			dwd >>= 1;
		}

		*((DWORD*)d) = dwd;

		setflagsuponval(d,l);
	}
}
