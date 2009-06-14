
#include "global.h"
#include "stackops.h"
#include "addrbus.h"
#include "memwin.h"
#include "cpustkw.h"
#include "cpuexec.h"

extern DWORD				exec_begin_cs,exec_begin_eip;

int do_not_log_sp=0;

void PushWord(DWORD v)
{
	DWORD o;
	DWORD osp;

	osp = ireg_esp;

// make sure changes show up in memory window
	o = ((ireg_ss&0xFFFF)<<4) + (ireg_esp&0xFFFF);
	if (o >= memory_win_begin && o <= memory_win_end)
		MemoryWinUpdate=TRUE;

	ireg_esp -= 2;
	ireg_esp &= 0xFFFF;
	writememwordfarptr(ireg_ss,ireg_esp,(WORD)v);

	if (!do_not_log_sp) AddStackWatchEntry(SWT_PUSH,ireg_cs,ireg_eip,exec_begin_cs,exec_begin_eip,ireg_ss,ireg_ss,osp,ireg_esp);
}

DWORD PopWord()
{
	DWORD w;
	DWORD o;
	DWORD osp;

	osp = ireg_esp;

// make sure changes show up in memory window
	o = ((ireg_ss&0xFFFF)<<4) + (ireg_esp&0xFFFF);
	if (o >= memory_win_begin && o <= memory_win_end)
		MemoryWinUpdate=TRUE;

	w = (DWORD)memwordfarptr(ireg_ss,ireg_esp);
	ireg_esp += 2;
	ireg_esp &= 0xFFFF;

// make sure changes show up in memory window
	o = ((ireg_ss&0xFFFF)<<4) + (ireg_esp&0xFFFF);
	if (o >= memory_win_begin && o <= memory_win_end)
		MemoryWinUpdate=TRUE;

	if (!do_not_log_sp) AddStackWatchEntry(SWT_POP,ireg_cs,ireg_eip,exec_begin_cs,exec_begin_eip,ireg_ss,ireg_ss,osp,ireg_esp);

	return w;
}

void PushDword(DWORD o)
{
	PushWord((WORD)(o&0xFFFF));
	PushWord((WORD)((o>>16)&0xFFFF));
}

DWORD PopDword()
{
	DWORD w;

	w = (((DWORD)PopWord())<<16);
	w |= (DWORD)PopWord();

	return w;
}

void InterruptFrameCall16(DWORD new_cs,DWORD new_ip)
{
	PushWord((WORD)(ireg_flags & 0xFFFF));
	PushWord((WORD)(ireg_cs & 0xFFFF));
	PushWord((WORD)(ireg_eip & 0xFFFF));
	ireg_cs = new_cs;
	ireg_eip = new_ip;
}

void InterruptFrameRet16()
{
	ireg_eip	 = PopWord();
	ireg_cs		&= 0xFFFF0000;
	ireg_cs		|= PopWord();
	ireg_flags	&= 0xFFFF0000;
	ireg_flags	|= PopWord();
}

void SignalInterrupt16(int num)
{
	num &= 255;
// call whatever handler is assigned to INT n
	InterruptFrameCall16(memwordlinear((num * 4) + 2),memwordlinear(num * 4));
	ireg_flags &= ~0x200;
}

void NearCall16(int newip)
{
	PushWord((WORD)(ireg_eip & 0xFFFF));
	ireg_eip = newip;
}

void FarCall16(int new_cs,int new_ip)
{
	PushWord((WORD)(ireg_cs & 0xFFFF));
	PushWord((WORD)(ireg_eip & 0xFFFF));
	ireg_cs = new_cs;
	ireg_eip = new_ip;
}
