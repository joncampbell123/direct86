/* 8086 emulation code and model table */
/* Just like the real thing, opcode 0Fh is treated as "POP CS" */

extern int do_not_log_sp;

int exec_add_or_sub_sp=0;

void Interpret386MODREGRM2(char *insname);
void Execute386MODREGRM2(void(*callfunc)(void*,void*,int));

#include "global.h"
#include <stdio.h>
#include "addrbus.h"
#include "hardware.h"
#include "cpustkw.h"
#include "cpudec.h"
#include "cpuexec.h"
#include "cpuqueue.h"
#include "cpumodel.h"
#include "execrm.h"
#include "execops.h"
#include "flagops.h"
#include "stackops.h"
#include "interrm.h"
#include "8086.h"
#include "brkpts.h"
#include "naming.h"

extern HWND hwndMain;

/* unknown instructions that somehow the CPU does not fault */

int cpu_exec_mystery_F1(BYTE b)		    { return 1; }
int cpu_dec_mystery_F1(BYTE b,char *buf){ strcpy(buf,"???");	return 1; }

/* flag operation instructions */

int cpu_exec_CMC(BYTE b)				{ ireg_flags ^= 1;		return 1; }
int cpu_dec_CMC(BYTE b,char *buf)		{ strcpy(buf,"CMC");	return 1; }

int cpu_exec_CLD(BYTE b)				{ ireg_flags &= ~0x400;	return 1; }
int cpu_dec_CLD(BYTE b,char *buf)		{ strcpy(buf,"CLD");	return 1; }

int cpu_exec_CLC(BYTE b)                { ireg_flags &= ~1;     return 1; }
int cpu_dec_CLC(BYTE b,char *buf)		{ strcpy(buf,"CLC");	return 1; }

int cpu_exec_CLI(BYTE b)                { ireg_flags &= ~0x200; return 1; }
int cpu_dec_CLI(BYTE b,char *buf)       { strcpy(buf,"CLI");    return 1; }

int cpu_exec_CWD(BYTE b)                { ireg_edx = (ireg_eax & 0x8000) ? 0xFFFF : 0x0000;	return 1; }
int cpu_dec_CWD(BYTE b,char *buf)       { strcpy(buf,"CWD");								return 1; }

int cpu_exec_HLT(BYTE b)				{ return 1; }
int cpu_dec_HLT(BYTE b,char *buf)		{ strcpy(buf,"HLT");	return 1; }

int cpu_exec_LAHF(BYTE b)               { ireg_eax = (ireg_eax & ~0xFF00) | ((ireg_flags & 0xFF) << 8); return 1; }
int cpu_dec_LAHF(BYTE b,char *buf)		{ strcpy(buf,"LAHF");	return 1; }

int cpu_exec_NOP(BYTE b)				{ return 1; }
int cpu_dec_NOP(BYTE b,char *buf)		{ strcpy(buf,"NOP");	return 1; }

int cpu_exec_PUSHF(BYTE b)              { PushWord((WORD)(ireg_flags&0xFFFF)); return 1; }
int cpu_dec_PUSHF(BYTE b,char *buf)		{ strcpy(buf,"PUSHF");	return 1; }

int cpu_exec_POPF(BYTE b)               { ireg_flags = (ireg_flags & ~0xFFFF) | PopWord(); return 1; }
int cpu_dec_POPF(BYTE b,char *buf)		{ strcpy(buf,"POPF");	return 1; }

int cpu_exec_SAHF(BYTE b)               { ireg_flags = (ireg_flags & ~0xFF) | ((ireg_eax >> 8) & 0xFF); return 1; }
int cpu_dec_SAHF(BYTE b,char *buf)		{ strcpy(buf,"SAHF");	return 1; }

int cpu_exec_STC(BYTE b)                { ireg_flags |= 1;      return 1; }
int cpu_dec_STC(BYTE b,char *buf)		{ strcpy(buf,"STC");	return 1; }

int cpu_exec_STD(BYTE b)                { ireg_flags |= 0x400;  return 1; }
int cpu_dec_STD(BYTE b,char *buf)		{ strcpy(buf,"STD");	return 1; }

int cpu_exec_STI(BYTE b)                { ireg_flags |= 0x200;  return 1; }
int cpu_dec_STI(BYTE b,char *buf)       { strcpy(buf,"STI");    return 1; }

int cpu_exec_WAIT(BYTE b)               { return 1; }
int cpu_dec_WAIT(BYTE b,char *buf)      { strcpy(buf,"WAIT");   return 1; }

/* conversion instructions */

int cpu_exec_CBW(BYTE b)
{
	if (ireg_eax & 0x80)
		ireg_eax |= 0xFF00;
	else
		ireg_eax &= 0x007F;

	return 1;
}

int cpu_dec_CBW(BYTE b,char *buf)
{
	strcpy(buf,"CBW");

	return 1;
}

/* prefixes */

int cpu_exec_REPZ(BYTE b)
{
    if (procREP) return 0;      /* invalid -- can't have successive REPs */

	REPLoop=TRUE;
	REPZflag=1;
	rep_loopaddr_cs = ireg_cs;
	rep_loopaddr_ip = ireg_eip-1;
	procREP = TRUE;

	return 1; 
}

int cpu_dec_REPZ(BYTE b,char *buf)
{
    strcpy(buf,"REP ");

	return 1;
}

int cpu_exec_REPNZ(BYTE b)
{
    if (procREP) return 0;      /* invalid -- can't have successive REPs */

	REPLoop=TRUE;
	REPZflag=0;
	rep_loopaddr_cs = ireg_cs;
	rep_loopaddr_ip = ireg_eip-1;
	procREP = TRUE;

	return 1; 
}

int cpu_dec_REPNZ(BYTE b,char *buf)
{
    strcpy(buf,"REPNE ");

	return 1;
}

/* string instructions */

int cpu_exec_STOS(BYTE b)
{
/* STOSB? */
    if (b == 0xAA) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
            if (ireg_ecx > 0) {
                ireg_ecx--;                     /* if we're in a REP STOSW loop, return to beginning of instruction */

                if (ireg_ecx > 0) {
                    FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
                    ireg_eip = oireg_eip;
                }
            }
        }

    /* peform STOSB to ES:DI */
        writemembytefarptr(ireg_es,ireg_edi,(BYTE)(ireg_eax & 0xFF));
    /* increment DI */
        if (ireg_flags & 0x400)     ireg_edi -= 0x0001;
        else                        ireg_edi += 0x0001;
        ireg_edi &= 0xFFFF;
        ireg_ecx &= 0xFFFF;
    }
/* STOSW? */
    else if (b == 0xAB) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
            if (ireg_ecx > 0) {
                ireg_ecx--;                     /* if we're in a REP STOSW loop, return to beginning of instruction */

                if (ireg_ecx > 0) {
                    FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
                    ireg_eip = oireg_eip;
                }
            }
        }

    /* peform STOSW to ES:DI */
        writememwordfarptr(ireg_es,ireg_edi,(WORD)(ireg_eax & 0xFFFF));
    /* increment DI */
        if (ireg_flags & 0x400)     ireg_edi -= 0x0002;
        else                        ireg_edi += 0x0002;
        ireg_edi &= 0xFFFF;
        ireg_ecx &= 0xFFFF;
    }

	return 1; 
}

int cpu_dec_STOS(BYTE b,char *buf)
{
    if (b == 0xAA)      strcpy(buf,"STOSB");
    if (b == 0xAB)      strcpy(buf,"STOSW");

	return 1;
}

int cpu_exec_LODS(BYTE b)
{
/* LODSB? */
    if (b == 0xAC) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
            if (ireg_ecx > 0) {
                ireg_ecx--;                     /* if we're in a REP LODSW loop, return to beginning of instruction */

                if (ireg_ecx > 0) {
                    FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
                    ireg_eip = oireg_eip;
                }
            }
        }

    /* peform LODSB from [segment]:SI */
        ireg_eax &= ~0xFF;
        ireg_eax |= (DWORD)membytefarptr(*ireg_dataseg,ireg_esi);
    /* increment SI */
        if (ireg_flags & 0x400)     ireg_esi -= 0x0001;
        else                        ireg_esi += 0x0001;
        ireg_esi &= 0xFFFF;
    }
/* LODSW? */
    else if (b == 0xAD) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
            if (ireg_ecx > 0) {
                ireg_ecx--;                     /* if we're in a REP LODSW loop, return to beginning of instruction */

                if (ireg_ecx > 0) {
                    FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
                    ireg_eip = oireg_eip;
                }
            }
        }

    /* peform LODSB from [segment]:SI */
        ireg_eax &= ~0xFFFF;
        ireg_eax |= (DWORD)memwordfarptr(*ireg_dataseg,ireg_esi);
    /* increment SI */
        if (ireg_flags & 0x400)     ireg_esi -= 0x0002;
        else                        ireg_esi += 0x0002;
        ireg_esi &= 0xFFFF;
    }

	return 1; 
}

int cpu_dec_LODS(BYTE b,char *buf)
{
    if (b == 0xAC)      strcpy(buf,"LODSB");
    if (b == 0xAD)      strcpy(buf,"LODSW");

	return 1;
}

int cpu_exec_MOVS(BYTE b)
{
    BYTE tb;
    WORD tw;

/* MOVSB? */
    if (b == 0xA4) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
            if (ireg_ecx > 0) {
                ireg_ecx--;                     /* if we're in a REP LODSW loop, return to beginning of instruction */

                if (ireg_ecx > 0) {
                    FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
                    ireg_eip = oireg_eip;
                }
            }
        }

    /* peform MOVSB from [segment]:SI to ES:DI */
        tb = membytefarptr(*ireg_dataseg,ireg_esi);
        writemembytefarptr(ireg_es,ireg_edi,tb);
    /* increment SI */
        if (ireg_flags & 0x400)     { ireg_esi -= 1; ireg_edi -= 1; }
        else                        { ireg_esi += 1; ireg_edi += 1; }
        ireg_esi &= 0xFFFF;
        ireg_edi &= 0xFFFF;
    }
/* MOVSW? */
    else if (b == 0xA5) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
            if (ireg_ecx > 0) {
                ireg_ecx--;                     /* if we're in a REP LODSW loop, return to beginning of instruction */

                if (ireg_ecx > 0) {
                    FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
                    ireg_eip = oireg_eip;
                }
            }
        }

    /* peform MOVSW from [segment]:SI to ES:DI */
        tw = memwordfarptr(*ireg_dataseg,ireg_esi);
        writememwordfarptr(ireg_es,ireg_edi,tw);
    /* increment SI */
        if (ireg_flags & 0x400)     { ireg_esi -= 2; ireg_edi -= 2; }
        else                        { ireg_esi += 2; ireg_edi += 2; }
        ireg_esi &= 0xFFFF;
        ireg_edi &= 0xFFFF;
    }

	return 1; 
}

int cpu_dec_MOVS(BYTE b,char *buf)
{
    if (b == 0xA4)      strcpy(buf,"MOVSB");
    if (b == 0xA5)      strcpy(buf,"MOVSW");

	return 1;
}

int cpu_exec_CMPS(BYTE b)
{
    BYTE tb,tb2;
    WORD tw,tw2;

/* CMPSB? */
    if (b == 0xA6) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
			if (!ireg_ecx) return 1;
            else if (ireg_ecx > 0) ireg_ecx--;
        }

    /* peform CMPSB from [segment]:SI to ES:DI */
        tb  = membytefarptr(*ireg_dataseg,ireg_esi);
        tb2 = membytefarptr(ireg_es,ireg_edi);
    /* increment SI */
        if (ireg_flags & 0x400)     { ireg_esi -= 1; ireg_edi -= 1; }
        else                        { ireg_esi += 1; ireg_edi += 1; }
        ireg_esi &= 0xFFFF;
        ireg_edi &= 0xFFFF;

		exec_CMP(&tb,&tb2,1);
		if (((ireg_flags >> 6) & 1) != (DWORD)REPZflag || !ireg_ecx) {
			REPLoop = FALSE;
		}
		else if (procREP) {
			FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
			ireg_eip = oireg_eip;
		}
    }
/* CMPSW? */
    if (b == 0xA7) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
			if (!ireg_ecx) return 1;
            else if (ireg_ecx > 0) ireg_ecx--;
        }

    /* peform CMPSB from [segment]:SI to ES:DI */
        tw  = memwordfarptr(*ireg_dataseg,ireg_esi);
        tw2 = memwordfarptr(ireg_es,ireg_edi);
    /* increment SI */
        if (ireg_flags & 0x400)     { ireg_esi -= 2; ireg_edi -= 2; }
        else                        { ireg_esi += 2; ireg_edi += 2; }
        ireg_esi &= 0xFFFF;
        ireg_edi &= 0xFFFF;

		exec_CMP(&tw,&tw2,1);
		if (((ireg_flags >> 6) & 1) != (DWORD)REPZflag || !ireg_ecx) {
			REPLoop = FALSE;
		}
		else if (procREP) {
			FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
			ireg_eip = oireg_eip;
		}
    }

	return 1; 
}

int cpu_dec_CMPS(BYTE b,char *buf)
{
    if (b == 0xA6)      strcpy(buf,"CMPSB");
    if (b == 0xA7)      strcpy(buf,"CMPSW");

	return 1;
}

int cpu_exec_SCAS(BYTE b)
{
    BYTE tb,tb2;
    WORD tw,tw2;

/* SCASB? */
    if (b == 0xAE) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
			if (!ireg_ecx) return 1;
            else if (ireg_ecx > 0) ireg_ecx--;
        }

    /* peform SCASB from ES:DI */
        tb  = membytefarptr(ireg_es,ireg_edi);
        tb2 = (BYTE)(ireg_eax&0xFF);
    /* increment DI */
        if (ireg_flags & 0x400)     { ireg_edi -= 1; }
        else                        { ireg_edi += 1; }
        ireg_edi &= 0xFFFF;

		exec_CMP(&tb,&tb2,1);
		if (((ireg_flags >> 6) & 1) != (DWORD)REPZflag || !ireg_ecx) {
			REPLoop = FALSE;
		}
		else if (procREP) {
			FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
			ireg_eip = oireg_eip;
		}
    }
/* SCASW? */
    if (b == 0xAF) {
        ireg_ecx &= 0xFFFF;

        if (procREP) {
			if (!ireg_ecx) return 1;
            else if (ireg_ecx > 0) ireg_ecx--;
        }

    /* peform SCASW from ES:DI */
        tw  = memwordfarptr(ireg_es,ireg_edi);
        tw2 = (WORD)(ireg_eax&0xFFFF);
    /* increment DI */
        if (ireg_flags & 0x400)     { ireg_edi -= 2; }
        else                        { ireg_edi += 2; }
        ireg_edi &= 0xFFFF;

		exec_CMP(&tw,&tw2,1);
		if (((ireg_flags >> 6) & 1) != (DWORD)REPZflag || !ireg_ecx) {
			REPLoop = FALSE;
		}
		else if (procREP) {
			FlushCPUQueue();            /* clear prefetch queue -- we're looping back */
			ireg_eip = oireg_eip;
		}
    }

	return 1; 
}

int cpu_dec_SCAS(BYTE b,char *buf)
{
    if (b == 0xAE)      strcpy(buf,"SCASB");
    if (b == 0xAF)      strcpy(buf,"SCASW");

	return 1;
}

int cpu_exec_CS_PR(BYTE b)
{
    if (segov) return 0;        /* invalid -- can't have successive segment overrides */

	segov=TRUE;
	ireg_dataseg = &ireg_cs;
	ireg_datasegreset=1;

	return 1; 
}

int cpu_dec_CS_PR(BYTE b,char *buf)
{
	strcpy(segment_prefix_dec,"CS:");
	buf[0] = 0;

	return 1;
}

int cpu_exec_DS_PR(BYTE b)
{
    if (segov) return 0;        /* invalid -- can't have successive segment overrides */

	segov=TRUE;
	ireg_dataseg = &ireg_ds;
	ireg_datasegreset=1;

	return 1; 
}

int cpu_dec_DS_PR(BYTE b,char *buf)
{
	strcpy(segment_prefix_dec,"DS:");
	buf[0] = 0;

	return 1;
}

int cpu_exec_ES_PR(BYTE b)
{
    if (segov) return 0;        /* invalid -- can't have successive segment overrides */

	segov=TRUE;
	ireg_dataseg = &ireg_es;
	ireg_datasegreset=1;

	return 1; 
}

int cpu_dec_ES_PR(BYTE b,char *buf)
{
	strcpy(segment_prefix_dec,"ES:");
	buf[0] = 0;

	return 1;
}

int cpu_exec_SS_PR(BYTE b)
{
    if (segov) return 0;        /* invalid -- can't have successive segment overrides */

	segov=TRUE;
	ireg_dataseg = &ireg_ss;
	ireg_datasegreset=1;

	return 1; 
}

int cpu_dec_SS_PR(BYTE b,char *buf)
{
	strcpy(segment_prefix_dec,"SS:");
	buf[0] = 0;

	return 1;
}

int cpu_exec_LOCK_PR(BYTE b)
{
    if (exec_buslock) return 0;     /* invalid -- can't have successive LOCKs */

	exec_buslockcnt=1;
	exec_buslock=TRUE;

	return 1; 
}

int cpu_dec_LOCK_PR(BYTE b,char *buf)
{
	strcpy(buf,"LOCK ");

	return 1;
}

/* STACK functions */

int cpu_exec_PUSHWORD_IMM(BYTE b)
{
	WORD w;

	if (b == 0x68) {
		w = (WORD)CPUQueueFetch();
		w |= ((WORD)CPUQueueFetch())<<8;
	}
	if (b == 0x6A) {
		w = (WORD)CPUQueueFetch();
		if (w & 0x80) w |= 0xFF80;
	}
	PushWord(w);

    return 1;
}

int cpu_dec_PUSHWORD_IMM(BYTE b,char *buf)
{
	WORD w;

	if (b == 0x68) {
		w = (WORD)CPUDecoderQueueFetch();
		w |= ((WORD)CPUDecoderQueueFetch())<<8;
	}
	if (b == 0x6A) {
		w = (WORD)CPUDecoderQueueFetch();
		if (w & 0x80) w |= 0xFF80;
	}
    sprintf(buf,"PUSH    %04X",w);

    return 1;
}

int cpu_exec_PUSHSEG(BYTE b)
{
    PushWord(*(regptr_seg[(b>>3)&3]));

    return 1;
}

int cpu_dec_PUSHSEG(BYTE b,char *buf)
{
    sprintf(buf,"PUSH    %s",regsegs[(b>>3)&3]);

    return 1;
}

int cpu_exec_POPSEG(BYTE b)
{
    *(regptr_seg[(b>>3)&3]) = (WORD)PopWord();

    return 1;
}

int cpu_dec_POPSEG(BYTE b,char *buf)
{
    sprintf(buf,"POP     %s",regsegs[(b>>3)&3]);

    return 1;
}

/* CALL instructions */

int cpu_exec_CALLFAR(BYTE b)
{
    WORD w,w2;

	do_not_log_sp=1;

	w2  = (WORD)CPUQueueFetch();
	w2 |= ((WORD)CPUQueueFetch())<<8;
	w   = (WORD)CPUQueueFetch();
	w  |= ((WORD)CPUQueueFetch())<<8;

	AddStackWatchEntry(SWT_CALLFAR,w,w2,ireg_cs,ireg_eip-5,ireg_ss,ireg_ss,ireg_esp,ireg_esp-4);

    FlushCPUQueue();            /* clear prefetch queue -- we're performing a call */
	FarCall16(w,w2);

	do_not_log_sp=0;

    return 1;
}

int cpu_dec_CALLFAR(BYTE b,char *buf)
{
    WORD w,w2;

	w2  = (WORD)CPUDecoderQueueFetch();
	w2 |= ((WORD)CPUDecoderQueueFetch())<<8;
	w   = (WORD)CPUDecoderQueueFetch();
	w  |= ((WORD)CPUDecoderQueueFetch())<<8;

    sprintf(buf,"CALL    %04X:%04X",w,w2);

	AppendCallProcName(w,w2,buf);

	return 1;
}

int cpu_exec_CALLNEAR(BYTE b)
{
    DWORD ofs;

	do_not_log_sp=1;

    ofs = (DWORD)CPUQueueFetch();
    ofs |= ((DWORD)CPUQueueFetch()) << 8;
    ofs += ireg_eip;
    ofs &= 0xFFFF;

	AddStackWatchEntry(SWT_CALLNEAR,ireg_cs,ofs,ireg_cs,ireg_eip-3,ireg_ss,ireg_ss,ireg_esp,ireg_esp-2);

    FlushCPUQueue();            /* clear prefetch queue -- we're performing a call */
    NearCall16(ofs);

	do_not_log_sp=0;

    return 1;
}

int cpu_dec_CALLNEAR(BYTE b,char *buf)
{
    DWORD ofs;

    ofs = (DWORD)CPUDecoderQueueFetch();
    ofs |= ((DWORD)CPUDecoderQueueFetch()) << 8;
    ofs += deci_eip;
    ofs &= 0xFFFF;

    sprintf(buf,"CALL    %04X",ofs);

	AppendCallProcName(ireg_cs,ofs,buf);

	return 1;
}

/* JMP instructions */

int cpu_exec_JMPFAR(BYTE b)
{
	WORD w,w2;

	w = (WORD)CPUQueueFetch();
	w |= ((WORD)CPUQueueFetch()) << 8;
	w2 = (WORD)CPUQueueFetch();
	w2 |= ((WORD)CPUQueueFetch()) << 8;
// direct jump to w2:w
    FlushCPUQueue();            /* clear prefetch queue -- we're performing a jump */
	ireg_cs = (int)w2;
	ireg_eip = (int)w;

    return 1;
}

int cpu_dec_JMPFAR(BYTE b,char *buf)
{
	WORD w,w2;

	w = (WORD)CPUDecoderQueueFetch();
	w |= ((WORD)CPUDecoderQueueFetch()) << 8;
	w2 = (WORD)CPUDecoderQueueFetch();
	w2 |= ((WORD)CPUDecoderQueueFetch()) << 8;

    sprintf(buf,"JMP     %04Xh:%04Xh",w2,w);

	AppendCallProcName(w2,w,buf);

	return 1;
}

int cpu_exec_JMPNEAR(BYTE b)
{
    DWORD ofs;

    ofs = (DWORD)CPUQueueFetch();
    ofs |= ((DWORD)CPUQueueFetch()) << 8;
    ofs += ireg_eip;
    ofs &= 0xFFFF;

    FlushCPUQueue();            /* clear prefetch queue -- we're performing a jump */
    ireg_eip = ofs;

    return 1;
}

int cpu_dec_JMPNEAR(BYTE b,char *buf)
{
    DWORD ofs;

    ofs = (DWORD)CPUDecoderQueueFetch();
    ofs |= ((DWORD)CPUDecoderQueueFetch()) << 8;
    ofs += deci_eip;
    ofs &= 0xFFFF;

    sprintf(buf,"JMP     %04Xh",ofs);

	AppendCallProcName(ireg_cs,ofs,buf);

	return 1;
}

int cpu_exec_JMPSHORT(BYTE b)
{
	BYTE b2;

	b2 = CPUQueueFetch();
    FlushCPUQueue();            /* clear prefetch queue -- we're performing a jump */
	ireg_eip += (int)((char)b2);

    return 1;
}

int cpu_dec_JMPSHORT(BYTE b,char *buf)
{
    DWORD ofs;

    ofs = (DWORD)CPUDecoderQueueFetch();
	if (ofs >= 0x80) ofs |= 0xFFFFFF80;
    ofs += deci_eip;
    ofs &= 0xFFFF;

    sprintf(buf,"JMP short %04Xh",ofs);

	AppendCallProcName(ireg_cs,ofs,buf);

	return 1;
}

/* logical operation instructions */

int cpu_exec_XLAT(BYTE b)
{
	WORD w;

	w = (WORD)(ireg_eax & 0xFF);
	w += (WORD)(ireg_ebx & 0xFFFF);	// Bizzare... but the way it apparently works as seen in DEBUG and CODEVIEW
	w &= 0xFFFF;
	ireg_eax &= ~0xFF;
	ireg_eax |= (DWORD)membytefarptr(*ireg_dataseg,(DWORD)w);

    return 1;
}

int cpu_dec_XLAT(BYTE b,char *buf)
{
	sprintf(buf,"XLAT");

    return 1;
}

int cpu_exec_TEST_R_AX(BYTE b)
{
	BYTE b1;
	WORD w1;

	if (b == 0xA8) {
		b1 = CPUQueueFetch();
		exec_TEST(&ireg_eax,&b1,1);
	}
	if (b == 0xA9) {
		w1 = (WORD)CPUQueueFetch();
		w1 |= ((WORD)CPUQueueFetch())<<8;
		exec_TEST(&ireg_eax,&w1,2);
	}

    return 1;
}

int cpu_dec_TEST_R_AX(BYTE b,char *buf)
{
	BYTE b1;
	WORD w1;

	if (b == 0xA8) {
		b1 = CPUDecoderQueueFetch();
		sprintf(buf,"TEST    AL,%02Xh",b1);
	}
	if (b == 0xA9) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"TEST    AX,%02Xh",w1);
	}

    return 1;
}

int cpu_exec_TEST(BYTE b)
{
    BYTE mrr;

    fl_d = 0;
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMODREGRM(exec_TEST);

    return 1;
}

int cpu_dec_TEST(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = 0;
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	InterpretMODREGRM("TEST");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_CMP(BYTE b)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	ExecuteMODREGRM(exec_CMP);

    return 1;
}

int cpu_dec_CMP(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	InterpretMODREGRM("CMP");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_CMP_R_AX(BYTE b)
{
	BYTE b1;
	WORD w1;

	if (b == 0x3C) {			// CMP al,imm
		b1 = CPUQueueFetch();
		exec_CMP(&b1,&ireg_eax,1);
	}
	if (b == 0x3D) {			// CMP ax,imm
		w1 = (WORD)CPUQueueFetch();
		w1 |= ((WORD)CPUQueueFetch())<<8;
		exec_CMP(&w1,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_CMP_R_AX(BYTE b,char *buf)
{
	BYTE b1;
	WORD w1;

	if (b == 0x3C) {			// CMP al,imm
		b1 = CPUDecoderQueueFetch();
		sprintf(buf,"CMP     AL,%02Xh",b1);
	}
	if (b == 0x3D) {			// CMP ax,imm
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"CMP     AX,%04Xh",w1);
	}

    return 1;
}

int cpu_exec_MOV_R_AX(BYTE b)
{
	WORD w;

	w = (WORD)CPUQueueFetch();
	w |= ((WORD)CPUQueueFetch())<<8;

	if (b == 0xA0) {
		ireg_eax &= ~0xFF;
		ireg_eax |= membytefarptr(*ireg_dataseg,(DWORD)w);
	}
	if (b == 0xA1) {
		ireg_eax &= ~0xFFFF;
		ireg_eax |= memwordfarptr(*ireg_dataseg,(DWORD)w);
	}
	if (b == 0xA2)
		writemembytefarptr(*ireg_dataseg,(DWORD)w,(BYTE)(ireg_eax&0xFF));
	if (b == 0xA3)
		writememwordfarptr(*ireg_dataseg,(DWORD)w,(WORD)(ireg_eax&0xFFFF));

    return 1;
}

int cpu_dec_MOV_R_AX(BYTE b,char *buf)
{
	WORD w;

	w = (WORD)CPUDecoderQueueFetch();
	w |= ((WORD)CPUDecoderQueueFetch())<<8;

	if (b == 0xA0)
		sprintf(buf,"MOV     AL,BYTE PTR %s[%04Xh]",segment_prefix_dec,w);
	if (b == 0xA1)
		sprintf(buf,"MOV     AX,WORD PTR %s[%04Xh]",segment_prefix_dec,w);
	if (b == 0xA2)
		sprintf(buf,"MOV     BYTE PTR %s[%04Xh],AL",segment_prefix_dec,w);
	if (b == 0xA3)
		sprintf(buf,"MOV     WORD PTR %s[%04Xh],AX",segment_prefix_dec,w);

    return 1;
}

int cpu_exec_DEC_REG_IR(BYTE b)
{
	exec_DEC(regptr_dec[b&7],NULL,2);

    return 1;
}

int cpu_dec_DEC_REG_IR(BYTE b,char *buf)
{
	sprintf(buf,"DEC     %s",regs16[b&7]);

    return 1;
}

int cpu_exec_INC_REG_IR(BYTE b)
{
	exec_INC(regptr_dec[b&7],NULL,2);

    return 1;
}

int cpu_dec_INC_REG_IR(BYTE b,char *buf)
{
	sprintf(buf,"INC     %s",regs16[b&7]);

    return 1;
}

int cpu_exec_IMUL_386(BYTE b)
{
    BYTE mrr;

	fl_d = (b>>1)&1;
	fl_w = (b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	if (b == 0x69)	Execute386MODREGRM1(exec_IMUL386);
	if (b == 0x6B)	Execute386MODREGRM2(exec_IMUL386);

    return 1;
}

int cpu_dec_IMUL_386(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    if (b == 0x69)	Interpret386MODREGRM1("IMUL");
    if (b == 0x6B)	Interpret386MODREGRM2("IMUL");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_AND_MRR(BYTE b)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMODREGRM(exec_AND);

    return 1;
}

int cpu_dec_AND_MRR(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("AND");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_AAA(BYTE b)
{
	WORD w;
	BYTE alo1,alo2;

/* What this instruction apparently does is set the carry flag, add 6
   to the lower nibble of AL and zero the high nibble of AL. AH is
   incremented, twice if the subtraction from AL causes a carry. */

    ireg_flags &= ~0x800;       /* overflow is always cleared */

/* 1-05-2001: High nibble of AL is always cleared */
	ireg_eax &= ~0xF0;

/* 1-05-2001: If the lower nibble >= 0xA, AUX and CARRY are set */
	if ((ireg_eax & 0xF) >= 0xA) ireg_flags |= 0x11;

/* 8-22-99: It seems Aux must be set or AAA does nothing */
	w = (WORD)(ireg_eax & 0xFFFF);
	if (ireg_flags & 0x10) {
/* 8-23-99: Apprently if AC is set CARRY is always set */
		ireg_flags |= 1;

		alo1 = (BYTE)(w & 0xF);
		alo2 = (BYTE)((w >> 8) & 0xFF);

		alo1 += 6;
		alo2++;
		if (alo1 &= 0xF0) {
			alo1 &= 0xF;
			alo2++;
		}
		alo2 &= 0xFF;

		w = (((WORD)(alo2 & 0xFF))<<8) | ((WORD)(alo1 & 0xFF));
	}
	else {
		ireg_flags &= ~1;		// and clears carry
	}

	setflagsuponval(&w,2);

	ireg_eax &= ~0xFFFF;
	ireg_eax |= (DWORD)w;

    return 1;
}

int cpu_dec_AAA(BYTE b,char *buf)
{
	sprintf(buf,"AAA");

    return 1;
}

int cpu_exec_AAS(BYTE b)
{
	WORD w;
	BYTE alo1,alo2;

/* What this instruction apparently does is set the carry flag, subtract 6
   from the lower nibble of AL and zero the high nibble of AL. AH is
   decremented, twice if the subtraction from AL causes a carry. */

    ireg_flags &= ~0x800;       /* overflow is always cleared */

/* 1-05-2001: High nibble of AL is always cleared */
	ireg_eax &= ~0xF0;

/* 1-05-2001: If the lower nibble >= 0xA, AUX and CARRY are set */
	if ((ireg_eax & 0xF) >= 0xA) ireg_flags |= 0x11;

/* 8-22-99: It seems Aux must be set or AAS does nothing */
	w = (WORD)(ireg_eax & 0xFFFF);
	if (ireg_flags & 0x10) {
/* 8-23-99: Apprently if AC is set CARRY is always set */
		ireg_flags |= 1;

		alo1 = (BYTE)(w & 0xF);
		alo2 = (BYTE)((w >> 8) & 0xFF);

		alo1 -= 6;
		alo2--;
		if (alo1 &= 0xF0) {
			alo1 &= 0xF;
			alo2--;
		}
		alo2 &= 0xFF;

		w = (((WORD)(alo2 & 0xFF))<<8) | ((WORD)(alo1 & 0xFF));
	}
	else {
		ireg_flags &= ~1;		// and clears carry
	}

	setflagsuponval(&w,2);

	ireg_eax &= ~0xFFFF;
	ireg_eax |= (DWORD)w;

    return 1;
}

int cpu_dec_AAS(BYTE b,char *buf)
{
	sprintf(buf,"AAS");

    return 1;
}

int cpu_exec_AAD(BYTE b)
{
	WORD w,w2;
	BYTE b2,b3;

// less known nowadays.... from the days of the
// old 8-bit microprocessors....
	w = (WORD)ireg_eax;
	b2 = CPUQueueFetch();
	w2 = ((w>>8) * ((WORD)b2));
	w2 &= 0xFF;
	w2 += (w & 0xFF);

	ireg_flags &= ~0x810; // overflow and aux are always cleared

	if (w2 & 0xFF00)		ireg_flags |= 0x11;
	else					ireg_flags &= ~0x11;

	ireg_eax &= ~0xFFFF;
	ireg_eax |= (DWORD)(w2 & 0xFF);
	b3 = (BYTE)w2;

	setflagsuponval(&b3,1);

    return 1;
}

int cpu_dec_AAD(BYTE b,char *buf)
{
	sprintf(buf,"AAD     %d",CPUDecoderQueueFetch());

    return 1;
}

int cpu_exec_AAM(BYTE b)
{
	WORD w;
	BYTE b2;
	unsigned int d1,d2;

	w = (WORD)(ireg_eax & 0xFF);
	b2 = CPUQueueFetch();

	ireg_flags &= ~0x811; // overflow, aux, and carry are always cleared

	d1 = (unsigned int)b2;
	d2 = ((unsigned int)w) / d1;
	d1 = ((unsigned int)w) % d1;

	w = (((WORD)(d2&0xFF))<<8) | ((WORD)(d1&0xFF));
	ireg_eax &= ~0xFFFF;
	ireg_eax |= (WORD)w;

	setflagsuponval(&d1,1);

    return 1;
}

int cpu_dec_AAM(BYTE b,char *buf)
{
	sprintf(buf,"AAM %d",CPUDecoderQueueFetch());

    return 1;
}

int cpu_exec_DAA(BYTE b)
{
	int digit1,digit2;
	BOOL carry,acarry;
	WORD w;

	ireg_flags &= ~0x800;	// overflow is always cleared

	carry=acarry=FALSE;
	w = (WORD)(ireg_eax & 0xFF);
	digit1 = (int)(w & 0xF);
	digit2 = (int)((w>>4) & 0xF);
	if (digit1 >= 0x0A) {
		digit2++;
		digit1 -= 0x0A;
		acarry=TRUE;
	}
	if (digit2 >= 0x0A) {
		digit2 -= 0x0A;
		carry=TRUE;
	}
// 8-23-99: Here's something strange: If carry is set, 6 is added to
// the second digit. If aux is set, 6 is added to the first digit.
	if (ireg_flags & 1)
		digit2 += 6;
	if (ireg_flags & 0x10)
		digit1 += 6;

	ireg_flags &= ~0x11;
	if (carry) ireg_flags |= 1;
	if (acarry) ireg_flags |= 0x10;
	w = (digit2 << 4) | digit1;
	setflagsuponval(&w,1);

    return 1;
}

int cpu_dec_DAA(BYTE b,char *buf)
{
	sprintf(buf,"DAA");

    return 1;
}

int cpu_exec_DAS(BYTE b)
{
	int digit1,digit2;
	BOOL carry,acarry;
	WORD w;

	ireg_flags &= ~0x800;	// overflow is always cleared

	carry=acarry=FALSE;
	w = (WORD)(ireg_eax & 0xFF);
	digit1 = (int)(w & 0xF);
	digit2 = (int)((w>>4) & 0xF);

	if ((ireg_flags & 1) || digit2 >= 0xA)
		digit2 -= 6;
	if ((ireg_flags & 0x10) || digit1 >= 0xA)
		digit1 -= 6;

	if (digit1 & 0xF0) {
		digit2--;
		digit1 &= 0xF;
		acarry=TRUE;
	}
	if (digit2 & 0xF0) {
		digit2 &= 0xF;
		carry=TRUE;
	}

	ireg_flags &= ~0x11;
	if (carry) ireg_flags |= 1;
	if (acarry) ireg_flags |= 0x10;
	w = (digit2 << 4) | digit1;
	setflagsuponval(&w,1);

    return 1;
}

int cpu_dec_DAS(BYTE b,char *buf)
{
	sprintf(buf,"DAS");

    return 1;
}

int cpu_exec_XCHG(BYTE b)
{
    BYTE mrr;

	fl_s = FALSE;
	fl_w = (b&1);
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMODREGRMXCHG(exec_XCHG);

    return 1;
}

int cpu_dec_XCHG(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("XCHG");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_XCHG_AX_REG(BYTE b)
{
	exec_XCHG(regptr_dec[b&7],&ireg_eax,2);

    return 1;
}

int cpu_dec_XCHG_AX_REG(BYTE b,char *buf)
{
    sprintf(buf,"XCHG    AX,%s",regs16[b&7]);

    return 1;
}

int cpu_exec_SUB_MRR(BYTE b)
{
    BYTE mrr;

	exec_add_or_sub_sp=1;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMODREGRM(exec_SUB);

    return 1;
}

int cpu_dec_SUB_MRR(BYTE b,char *buf)
{
    BYTE mrr;

	fl_d = (b>>1)&1;
	fl_w = (b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("SUB");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_SUB_R_AX(BYTE b)
{
	BYTE b2;
	WORD w;

	exec_add_or_sub_sp=1;

	if (b == 0x2C) {
		b2 = CPUQueueFetch();
		exec_SUB(&b2,&ireg_eax,1);
	}
	if (b == 0x2D) {
		w = (WORD)CPUQueueFetch();
		w |= ((WORD)CPUQueueFetch())<<8;
		exec_SUB(&w,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_SUB_R_AX(BYTE b,char *buf)
{
	BYTE b2;
	WORD w;

	if (b == 0x2C) {
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"SUB     AL,%02Xh",b2);
	}
	if (b == 0x2D) {
		w = (WORD)CPUDecoderQueueFetch();
		w |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"SUB     AX,%04Xh",w);
	}

    return 1;
}

int cpu_exec_ADD_MRR(BYTE b)
{
    BYTE mrr;

	exec_add_or_sub_sp=1;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMODREGRM(exec_ADD);

    return 1;
}

int cpu_dec_ADD_MRR(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("ADD");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_ADD_R_AX(BYTE b)
{
	BYTE b2;
	WORD w;

	exec_add_or_sub_sp=1;

	if (b == 0x04) {
		b2 = CPUQueueFetch();
		exec_ADD(&b2,&ireg_eax,1);
	}
	if (b == 0x05) {
		w = (WORD)CPUQueueFetch();
		w |= ((WORD)CPUQueueFetch())<<8;
		exec_ADD(&w,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_ADD_R_AX(BYTE b,char *buf)
{
	BYTE b2;
	WORD w;

	if (b == 0x04) {
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"ADD     AL,%02Xh",b2);
	}
	if (b == 0x05) {
		w = (WORD)CPUDecoderQueueFetch();
		w |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"ADD     AX,%04Xh",w);
	}

    return 1;
}

int cpu_exec_ADC_MRR(BYTE b)
{
    BYTE mrr;

	exec_add_or_sub_sp=1;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMODREGRM(exec_ADC);

    return 1;
}

int cpu_dec_ADC_MRR(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("ADC");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_AND_R_AX(BYTE b)
{
    BYTE b1;
	WORD w1;

	if (b == 0x24) {
	    b1 = CPUQueueFetch();
		exec_AND(&b1,&ireg_eax,1);
	}
	if (b == 0x25) {
	    w1 = (WORD)CPUQueueFetch();
	    w1 |= ((WORD)CPUQueueFetch())<<8;
		exec_AND(&w1,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_AND_R_AX(BYTE b,char *buf)
{
    BYTE b1;
	WORD w1;

	if (b == 0x24) {
	    b1 = CPUDecoderQueueFetch();
		sprintf(buf,"AND     AL,%02Xh",b1);
	}
	if (b == 0x25) {
	    w1 = (WORD)CPUDecoderQueueFetch();
	    w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"AND     AX,%04Xh",w1);
	}

    return 1;
}

int cpu_exec_ADC_R_AX(BYTE b)
{
    BYTE b1;
	WORD w1;

	exec_add_or_sub_sp=1;

	if (b == 0x14) {
	    b1 = CPUQueueFetch();
		exec_ADC(&b1,&ireg_eax,1);
	}
	if (b == 0x15) {
	    w1 = (WORD)CPUQueueFetch();
	    w1 |= ((WORD)CPUQueueFetch())<<8;
		exec_ADC(&w1,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_ADC_R_AX(BYTE b,char *buf)
{
    BYTE b1;
	WORD w1;

	if (b == 0x14) {
	    b1 = CPUDecoderQueueFetch();
		sprintf(buf,"ADC     AL,%02Xh",b1);
	}
	if (b == 0x15) {
	    w1 = (WORD)CPUDecoderQueueFetch();
	    w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"ADC     AX,%04Xh",w1);
	}

    return 1;
}

int cpu_exec_XOR_MRR(BYTE b)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
    ExecuteMODREGRM(exec_XOR);

    return 1;
}

int cpu_dec_XOR_MRR(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("XOR");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_XOR_R_AX(BYTE b)
{
    BYTE b1;
	WORD w1;

	if (b == 0x34) {
		b1 = CPUQueueFetch();
		exec_XOR(&b1,&ireg_eax,1);
	}
	if (b == 0x35) {
		w1 = (WORD)CPUQueueFetch();
		w1 |= ((WORD)CPUQueueFetch())<<8;
		exec_XOR(&w1,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_XOR_R_AX(BYTE b,char *buf)
{
    BYTE b1;
	WORD w1;

	if (b == 0x24) {
	    b1 = CPUDecoderQueueFetch();
		sprintf(buf,"XOR     AL,%02Xh",b1);
	}
	if (b == 0x25) {
	    w1 = (WORD)CPUDecoderQueueFetch();
	    w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"XOR     AX,%04Xh",w1);
	}

    return 1;
}

int cpu_exec_SBB_MRR(BYTE b)
{
    BYTE mrr;

	exec_add_or_sub_sp=1;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
    ExecuteMODREGRM(exec_SBB);

    return 1;
}

int cpu_dec_SBB_MRR(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("SBB");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_SBB_R_AX(BYTE b)
{
	BYTE b2;
	WORD w;

	exec_add_or_sub_sp=1;

	if (b == 0x1C) {
		b2 = CPUQueueFetch();
		exec_SBB(&b2,&ireg_eax,1);
	}
	if (b == 0x1D) {
		w = (WORD)CPUQueueFetch();
		w |= ((WORD)CPUQueueFetch())<<8;
		exec_SBB(&w,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_SBB_R_AX(BYTE b,char *buf)
{
	BYTE b2;
	WORD w;

	if (b == 0x1C) {
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"SBB     AL,%02Xh",b2);
	}
	if (b == 0x1D) {
		w = (WORD)CPUDecoderQueueFetch();
		w |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"SBB     AX,%04Xh",w);
	}

    return 1;
}

int cpu_exec_MOV_MRR(BYTE b)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
    ExecuteMODREGRM(exec_MOV);

    return 1;
}

int cpu_dec_MOV_MRR(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("MOV");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_MOV_MRR_C6(BYTE b)
{
    BYTE mrr;

	fl_w = (b&1);
	fl_fop = b;
	fl_s = FALSE;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMOD010RM(exec_MOV);

    return 1;
}

int cpu_dec_MOV_MRR_C6(BYTE b,char *buf)
{
    BYTE mrr;

	fl_w = (b&1);
	fl_fop = b;
	fl_s = FALSE;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMOD010RM("MOV");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_OR_MRR(BYTE b)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
    ExecuteMODREGRM(exec_OR);

    return 1;
}

int cpu_dec_OR_MRR(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODREGRM("OR");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_OR_R_AX(BYTE b)
{
	BYTE b2;
	WORD w;

	if (b == 0x0C) {
		b2 = CPUQueueFetch();
		exec_OR(&b2,&ireg_eax,1);
	}
	if (b == 0x0D) {
		w = (WORD)CPUQueueFetch();
		w |= ((WORD)CPUQueueFetch())<<8;
		exec_OR(&w,&ireg_eax,2);
	}

    return 1;
}

int cpu_dec_OR_R_AX(BYTE b,char *buf)
{
	BYTE b2;
	WORD w;

	if (b == 0x0C) {
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"OR      AL,%02X",b2);
	}
	if (b == 0x0D) {
		w = (WORD)CPUDecoderQueueFetch();
		w |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"OR      AX,%04X",w);
	}

    return 1;
}

int cpu_exec_MOV_MRRSEG(BYTE b)
{
    BYTE mrr;

	fl_d = (b>>1)&1;
	fl_w = TRUE;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	ExecuteMODSREGRM(exec_MOV);

    return 1;
}

int cpu_dec_MOV_MRRSEG(BYTE b,char *buf)
{
    BYTE mrr;

    fl_d = (int)((b>>1)&1);
    fl_w = TRUE;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

/* put through our standard mod-reg-rm decoder */

    InterpretMODSREGRM("MOV");
    strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_MOV_BOREGIMM(BYTE b)
{
	WORD w1;

	fl_w = (b>>3)&1;
	fl_reg = b&7;

	if (fl_w) {
		w1 = (WORD)CPUQueueFetch();
		w1 |= ((WORD)CPUQueueFetch())<<8;
		*(regptr_dec[fl_reg]) = w1;
	}
	else {
		*(regptr_decbyte[fl_reg]) = CPUQueueFetch();
	}

    return 1;
}

int cpu_dec_MOV_BOREGIMM(BYTE b,char *buf)
{
	WORD w1;
	BYTE b1;

	fl_w = (b>>3)&1;
	fl_reg = b&7;

	if (fl_w) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"MOV     %s,%04X",regs16[fl_reg],w1);
	}
	else {
		b1 = CPUDecoderQueueFetch();
		sprintf(buf,"MOV     %s,%02X",regs8[fl_reg],b1);
	}

    return 1;
}

/* BATCH sets */

int cpu_exec_batch_80(BYTE b)
{
    BYTE mrr;

	fl_fop = b;
    fl_s = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0x00)	ExecuteMOD010RM(exec_ADD);
	if (fl_reg == 0x01)	ExecuteMOD010RM(exec_OR);
	if (fl_reg == 0x02)	ExecuteMOD010RM(exec_ADC);
	if (fl_reg == 0x03)	ExecuteMOD010RM(exec_SBB);
	if (fl_reg == 0x04)	ExecuteMOD010RM(exec_AND);
	if (fl_reg == 0x05)	ExecuteMOD010RM(exec_SUB);
	if (fl_reg == 0x06)	ExecuteMOD010RM(exec_XOR);
	if (fl_reg == 0x07)	ExecuteMOD010RM(exec_CMP);

	if (fl_reg == 0 || fl_reg == 2 || fl_reg == 3 || fl_reg == 5) exec_add_or_sub_sp=1;

    return 1;
}

int cpu_dec_batch_80(BYTE b,char *buf)
{
    BYTE mrr;

	fl_fop = b;
    fl_s = (int)((b>>1)&1);
    fl_w = (int)(b&1);

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0x00)	InterpretMOD010RM("ADD");
	if (fl_reg == 0x01)	InterpretMOD010RM("OR");
	if (fl_reg == 0x02)	InterpretMOD010RM("ADC");
	if (fl_reg == 0x03)	InterpretMOD010RM("SBB");
	if (fl_reg == 0x04)	InterpretMOD010RM("AND");
	if (fl_reg == 0x05)	InterpretMOD010RM("SUB");
	if (fl_reg == 0x06)	InterpretMOD010RM("XOR");
	if (fl_reg == 0x07)	InterpretMOD010RM("CMP");

	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_batch_F6(BYTE b)
{
    BYTE mrr;

	fl_w = b&1;
	fl_s = FALSE;
	fl_d = TRUE;
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0x00)	ExecuteMOD010RM(exec_TEST);
	if (fl_reg == 0x01)	return 0;
	if (fl_reg == 0x02)	ExecuteMODS1RM(exec_NOT);
	if (fl_reg == 0x03)	ExecuteMODS1RM(exec_NEG);
	if (fl_reg == 0x04)	ExecuteMODS1RM(exec_MUL);
	if (fl_reg == 0x05)	ExecuteMODS1RM(exec_IMUL);
	if (fl_reg == 0x06)	ExecuteMODS1RM(exec_DIV);
	if (fl_reg == 0x07)	ExecuteMODS1RM(exec_IDIV);

    return 1;
}

int cpu_dec_batch_F6(BYTE b,char *buf)
{
    BYTE mrr;

	fl_w = b&1;
	fl_s = FALSE;
	fl_d = TRUE;
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0x00)	InterpretMOD010RM("TEST");
	if (fl_reg == 0x01)	InterpretMOD010RM("[unknown fl_reg 1]");
	if (fl_reg == 0x02)	InterpretMODS1RM("NOT");
	if (fl_reg == 0x03)	InterpretMODS1RM("NEG");
	if (fl_reg == 0x04)	InterpretMODS1RM("MUL");
	if (fl_reg == 0x05)	InterpretMODS1RM("IMUL");
	if (fl_reg == 0x06)	InterpretMODS1RM("DIV");
	if (fl_reg == 0x07)	InterpretMODS1RM("IDIV");

	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_batch_FE(BYTE b)
{
    BYTE mrr;

	fl_w = b&1;
	fl_s = FALSE;
	fl_d = TRUE;
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0) ExecuteMODS1RM(exec_INC);
	if (fl_reg == 1) ExecuteMODS1RM(exec_DEC);
	if (fl_reg == 2) ExecuteMODS1RMRDONLY(exec_CALL);
	if (fl_reg == 3) ExecuteMODS1RMDRDONLY(exec_FARCALL);
	if (fl_reg == 4) ExecuteMODS1RMRDONLY(exec_JMP);
	if (fl_reg == 5) ExecuteMODS1RMDRDONLY(exec_FARJMP);
	if (fl_reg == 6) ExecuteMODS1RMRDONLY(exec_PUSH);
	if (fl_reg == 7) return 0;

	if (fl_reg == 0 || fl_reg == 1) exec_add_or_sub_sp=1;

    return 1;
}

int cpu_dec_batch_FE(BYTE b,char *buf)
{
    BYTE mrr;

	fl_w = b&1;
	fl_s = FALSE;
	fl_d = TRUE;
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0) InterpretMODS1RM("INC");
	if (fl_reg == 1) InterpretMODS1RM("DEC");
	if (fl_reg == 2) InterpretMODS1RM("CALL");
	if (fl_reg == 3) InterpretMODS1RM("CALL");
	if (fl_reg == 4) InterpretMODS1RM("JMP");
	if (fl_reg == 5) InterpretMODS1RM("JMP");
	if (fl_reg == 6) InterpretMODS1RM("PUSH");
	if (fl_reg == 7) strcpy(intcbuf,"???");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_batch_C0(BYTE b)
{
    BYTE mrr;

	fl_s = FALSE;
	fl_w = (b&1);
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	if (fl_reg == 0x00)	ExecuteMODSHII(exec_ROL_imm);
	if (fl_reg == 0x01)	ExecuteMODSHII(exec_ROR_imm);
	if (fl_reg == 0x02)	ExecuteMODSHII(exec_RCL_imm);
	if (fl_reg == 0x03)	ExecuteMODSHII(exec_RCR_imm);
	if (fl_reg == 0x04)	ExecuteMODSHII(exec_SHL_imm);
	if (fl_reg == 0x05)	ExecuteMODSHII(exec_SHR_imm);
	if (fl_reg == 0x07)	ExecuteMODSHII(exec_SAR_imm);

    return 1;
}

int cpu_dec_batch_C0(BYTE b,char *buf)
{
    BYTE mrr;

	fl_s = FALSE;
	fl_w = (b&1);
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0x00)	InterpretMODSCLRM("ROL");
	if (fl_reg == 0x01)	InterpretMODSCLRM("ROR");
	if (fl_reg == 0x02)	InterpretMODSCLRM("RCL");
	if (fl_reg == 0x03)	InterpretMODSCLRM("RCR");
	if (fl_reg == 0x04)	InterpretMODSCLRM("SHL");
	if (fl_reg == 0x05)	InterpretMODSCLRM("SHR");
	if (fl_reg == 0x07)	InterpretMODSCLRM("SAR");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_batch_D0(BYTE b)
{
    BYTE mrr;

	fl_s = FALSE;
	fl_w = (b&1);
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0)	ExecuteMODS1RM(exec_ROL);
	if (fl_reg == 1)	ExecuteMODS1RM(exec_ROR);
	if (fl_reg == 2)	ExecuteMODS1RM(exec_RCL);
	if (fl_reg == 3)	ExecuteMODS1RM(exec_RCR);
	if (fl_reg == 4)	ExecuteMODS1RM(exec_SHL);
	if (fl_reg == 5)	ExecuteMODS1RM(exec_SHR);
	if (fl_reg == 7)	ExecuteMODS1RM(exec_SAR);

    return 1;
}

int cpu_dec_batch_D0(BYTE b,char *buf)
{
    BYTE mrr;

	fl_s = FALSE;
	fl_w = (b&1);
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0)	InterpretMODS1BRM("ROL");
	if (fl_reg == 1)	InterpretMODS1BRM("ROR");
	if (fl_reg == 2)	InterpretMODS1BRM("RCL");
	if (fl_reg == 3)	InterpretMODS1BRM("RCR");
	if (fl_reg == 4)	InterpretMODS1BRM("SHL");
	if (fl_reg == 5)	InterpretMODS1BRM("SHR");
	if (fl_reg == 7)	InterpretMODS1BRM("SAR");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_batch_D2(BYTE b)
{
    BYTE mrr;

	fl_s = FALSE;
	fl_w = (b&1);
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0)	ExecuteMODS1RM(exec_ROR_CL);
	if (fl_reg == 1)	ExecuteMODS1RM(exec_ROL_CL);
	if (fl_reg == 2)	ExecuteMODS1RM(exec_RCL_CL);
	if (fl_reg == 3)	ExecuteMODS1RM(exec_RCR_CL);
	if (fl_reg == 4)	ExecuteMODS1RM(exec_SHL_CL);
	if (fl_reg == 5)	ExecuteMODS1RM(exec_SHR_CL);
	if (fl_reg == 7)	ExecuteMODS1RM(exec_SAR_CL);

    return 1;
}

int cpu_dec_batch_D2(BYTE b,char *buf)
{
    BYTE mrr;

	fl_s = FALSE;
	fl_w = (b&1);
	fl_fop = b;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	if (fl_reg == 0)	InterpretMODSCLRM("ROR");
	if (fl_reg == 1)	InterpretMODSCLRM("ROL");
	if (fl_reg == 2)	InterpretMODSCLRM("RCL");
	if (fl_reg == 3)	InterpretMODSCLRM("RCR");
	if (fl_reg == 4)	InterpretMODSCLRM("SHL");
	if (fl_reg == 5)	InterpretMODSCLRM("SHR");
	if (fl_reg == 7)	InterpretMODSCLRM("SAR");
	strcpy(buf,intcbuf);

    return 1;
}

/* CPU calling stack operations */

int cpu_exec_RET(BYTE b)
{
	WORD w1;
	DWORD dd,dd2;

	do_not_log_sp=1;

	if (b == 0xC3) {			// RET?
		dd = PopWord();
		AddStackWatchEntry(SWT_RET,ireg_cs,dd,ireg_cs,ireg_eip-1,ireg_ss,ireg_ss,ireg_esp-2,ireg_esp);

		ireg_eip = dd;
        FlushCPUQueue();
	}
	if (b == 0xC2) {			// RET and pop n bytes?
		w1 = (WORD)CPUQueueFetch();
		w1 |= ((WORD)CPUQueueFetch())<<8;
		dd  = PopWord();
		AddStackWatchEntry(SWT_RET,ireg_cs,dd,ireg_cs,ireg_eip-3,ireg_ss,ireg_ss,ireg_esp-2,ireg_esp);

		dd2 = ireg_esp;
		ireg_esp = (ireg_esp + ((int)w1))&0xFFFF;
		AddStackWatchEntry(SWT_POPIGNORE,ireg_cs,dd,ireg_cs,ireg_eip-3,ireg_ss,ireg_ss,dd2,ireg_esp);

		ireg_eip = dd;
        FlushCPUQueue();
	}

	do_not_log_sp=0;

    return 1;
}

int cpu_dec_RET(BYTE b,char *buf)
{
	WORD w1;

	if (b == 0xC3) {			// RET?
		sprintf(buf,"RET");
	}
	if (b == 0xC2) {			// RET and pop n bytes?
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"RET     %d",w1);
	}

    return 1;
}

int cpu_exec_RETF(BYTE b)
{
	WORD w1;
	DWORD dd,dd2,dd3;

	do_not_log_sp=1;

	if (b == 0xCB) {			// RETF?
		dd  = PopWord();
		dd2 = PopWord();

		AddStackWatchEntry(SWT_RETF,dd2,dd,ireg_cs,ireg_eip-1,ireg_ss,ireg_ss,ireg_esp,ireg_esp+4);
		ireg_eip = dd;
		ireg_cs  = dd2;
	}
	if (b == 0xCA) {			// RETF and pop n bytes?
		w1 = (WORD)CPUQueueFetch();
		w1 |= ((WORD)CPUQueueFetch())<<8;
		dd  = PopWord();
		dd2 = PopWord();
		AddStackWatchEntry(SWT_RETF,dd2,dd,ireg_cs,ireg_eip-3,ireg_ss,ireg_ss,ireg_esp-4,ireg_esp);

		dd3 = ireg_esp;
		ireg_esp = (ireg_esp + ((int)w1))&0xFFFF;
		AddStackWatchEntry(SWT_POPIGNORE,dd2,dd,ireg_cs,ireg_eip-3,ireg_ss,ireg_ss,dd3,ireg_esp);

		ireg_eip = dd;
		ireg_cs  = dd2;
	}

	do_not_log_sp=0;

    return 1;
}

int cpu_dec_RETF(BYTE b,char *buf)
{
	WORD w1;

	if (b == 0xCB) {			// RETF?
		sprintf(buf,"RETF");
	}
	if (b == 0xCA) {			// RETF and pop n bytes?
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(buf,"RETF    %d",w1);
	}

    return 1;
}

int cpu_exec_IRET(BYTE b)
{
	WORD b2;
	DWORD ocs,oip,osp;

	do_not_log_sp=1;

	if (b == 0xCF) {			// IRET?
		ocs = ireg_cs;
		oip = ireg_eip;
		InterruptFrameRet16();
		AddStackWatchEntry(SWT_IRET,ireg_cs,ireg_eip,ocs,oip,ireg_ss,ireg_ss,ireg_esp-6,ireg_esp);
	}
	if (b == 0xCE) {			// IRET and pop n bytes?
		ocs = ireg_cs;
		oip = ireg_eip;
		b2 = CPUQueueFetch();
		InterruptFrameRet16();
		AddStackWatchEntry(SWT_IRET,ireg_cs,ireg_eip,ocs,oip,ireg_ss,ireg_ss,ireg_esp-6,ireg_esp);

		osp = ireg_esp;
		ireg_esp = ((ireg_esp + b2)&0xFFFF) | (ireg_esp & 0xFFFF0000);
		AddStackWatchEntry(SWT_POPIGNORE,ireg_cs,ireg_eip,ocs,oip,ireg_ss,ireg_ss,osp,ireg_esp);
	}

	do_not_log_sp=0;

    return 1;
}

int cpu_dec_IRET(BYTE b,char *buf)
{
	BYTE b2;

	if (b == 0xCF) {			// IRET?
		sprintf(buf,"IRET");
	}
	if (b == 0xCE) {			// IRET and pop n bytes?
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"IRET    %d",b2);
	}

    return 1;
}

int cpu_exec_PUSH_REG_IR(BYTE b)
{
	fl_reg = b&7;

	if ((b&7) == 5 && cpu_revision < 3)		PushWord((*(regptr_dec32[b&7])) - 4);
	else									PushWord(*(regptr_dec32[b&7]));

    return 1;
}

int cpu_dec_PUSH_REG_IR(BYTE b,char *buf)
{
	fl_reg = b&7;

	sprintf(buf,"PUSH    %s",regs16[fl_reg]);

    return 1;
}

int cpu_exec_POP_REG_IR(BYTE b)
{
	*(regptr_dec[b&7]) = (WORD)PopWord();

    return 1;
}

int cpu_dec_POP_REG_IR(BYTE b,char *buf)
{
	sprintf(buf,"POP     %s",regs16[b&7]);

    return 1;
}

int cpu_exec_POP_MEM_286(BYTE b)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	ExecuteMODS1RM(exec_POP);

    return 1;
}

int cpu_dec_POP_MEM_286(BYTE b,char *buf)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);

	InterpretMODS1RM("POP");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_PUSHA(BYTE b)
{
// on 386 processors and above (as referred to in Intel's 80386
// documentation) the execution of PUSH SP handled differently.
// apparently, the 808386 likes to store the value of SP before
// it pushes it onto the stack, as compared to the 8086 and (I
// assume) the 80286 processors who increment the value of SP
// and then store it. weird. And people wonder why some older
// DOS programs have problems on newer chips.
	DWORD old_esp = ireg_esp;

	if (edb66) {
		PushDword(ireg_eax);
		PushDword(ireg_ecx);
		PushDword(ireg_ebx);
		PushDword(ireg_edx);
		if (cpu_revision >= 3)	PushDword(old_esp);
		else					PushDword(ireg_esp);
		PushDword(ireg_ebp);
		PushDword(ireg_esi);
		PushDword(ireg_edi);
	}
	else {
		PushWord((WORD)(ireg_eax&0xFFFF));
		PushWord((WORD)(ireg_ecx&0xFFFF));
		PushWord((WORD)(ireg_ebx&0xFFFF));
		PushWord((WORD)(ireg_edx&0xFFFF));
		if (cpu_revision >= 3)	PushWord((WORD)(old_esp&0xFFFF));
		else					PushWord((WORD)(ireg_esp&0xFFFF));
		PushWord((WORD)(ireg_ebp&0xFFFF));
		PushWord((WORD)(ireg_esi&0xFFFF));
		PushWord((WORD)(ireg_edi&0xFFFF));
	}

    return 1;
}

int cpu_dec_PUSHA(BYTE b,char *buf)
{
	sprintf(buf,"PUSHA");

    return 1;
}

int cpu_exec_POPA(BYTE b)
{
// on 386 processors and above (as referred to in Intel's 80386
// documentation) the execution of PUSH SP handled differently.
// apparently, the 808386 likes to store the value of SP before
// it pushes it onto the stack, as compared to the 8086 and (I
// assume) the 80286 processors who increment the value of SP
// and then store it. weird. And people wonder why some older
// DOS programs have problems on newer chips.

	if (edb66) {
		ireg_edi = PopDword();
		ireg_esi = PopDword();
		ireg_ebp = PopDword();
		PopDword(); // Ignore SP just like the real thing
		ireg_edx = PopDword();
		ireg_ebx = PopDword();
		ireg_ecx = PopDword();
		ireg_eax = PopDword();
	}
	else {
// EDI
		ireg_edi &= ~0xFFFF;
		ireg_edi |= PopWord();
// ESI
		ireg_esi &= ~0xFFFF;
		ireg_esi |= PopWord();
// EBP
		ireg_ebp &= ~0xFFFF;
		ireg_ebp |= PopWord();
// ESP
		PopWord();		// SP is ignored
// EDX
		ireg_edx &= ~0xFFFF;
		ireg_edx |= PopWord();
// EBX
		ireg_ebx &= ~0xFFFF;
		ireg_ebx |= PopWord();
// ECX
		ireg_ecx &= ~0xFFFF;
		ireg_ecx |= PopWord();
// EAX
		ireg_eax &= ~0xFFFF;
		ireg_eax |= PopWord();
	}

    return 1;
}

int cpu_dec_POPA(BYTE b,char *buf)
{
	sprintf(buf,"POPA");

    return 1;
}

/* s/w interrupt generation */
int cpu_exec_INT(BYTE b)
{
	BYTE b2;
	DWORD oip,ocs;

	do_not_log_sp=1;

	if (b == 0xCD) {			// INT imm
		b2 = CPUQueueFetch();
		oip = ireg_eip;
		ocs = ireg_cs;
		SignalInterrupt16(b2);
		AddStackWatchEntryINT(SWT_CALLINT,ireg_cs,ireg_eip,ocs,oip,ireg_ss,ireg_ss,ireg_esp+6,ireg_esp,(int)b2);
		CheckTriggerSWINTBreakpoint((int)b2);
	}
	if (b == 0xCC) {			// INT 3
		oip = ireg_eip;
		ocs = ireg_cs;
		SignalInterrupt16(3);
		AddStackWatchEntryINT(SWT_CALLINT,ireg_cs,ireg_eip,ocs,oip,ireg_ss,ireg_ss,ireg_esp+6,ireg_esp,3);
		CheckTriggerSWINTBreakpoint(3);
	}

	do_not_log_sp=0;

    return 1;
}

int cpu_dec_INT(BYTE b,char *buf)
{
	BYTE b2;

	if (b == 0xCD) {			// INT imm
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"INT     %02Xh",b2);
	}
	if (b == 0xCC) {			// INT 3
		sprintf(buf,"INT     3");
	}

    return 1;
}

/* conditional jumping */
int cpu_exec_LOOP(BYTE b)
{
	BYTE b2;
	DWORD relav;
	int cnd;

	b2 = b&15;
	relav = (DWORD)CPUQueueFetch(); FlushCPUQueue();
	if (relav >= 0x80) relav |= 0xFFFFFF80;

	ireg_ecx = (ireg_ecx--)&0xFFFF;

	if (b == 0xE0)		cnd = (ireg_ecx&0xFFFF) && !(ireg_flags & 0x40);
	if (b == 0xE1)		cnd = (ireg_ecx&0xFFFF) && (ireg_flags & 0x40);
	if (b == 0xE2)		cnd = ireg_ecx&0xFFFF;

	if (cnd) ireg_eip = (ireg_eip + relav)&0xFFFF;

    return 1;
}

int cpu_dec_LOOP(BYTE b,char *buf)
{
	BYTE b2;
	int relav;

	b2 = b&15;
	relav = (int)CPUDecoderQueueFetch();
	if (relav >= 0x80) relav |= 0xFFFFFF80;

	if (b == 0xE0)		sprintf(buf,"LOOPNZ  %04Xh",(relav+deci_eip)&0xFFFF);
	if (b == 0xE1)		sprintf(buf,"LOOPZ   %04Xh",(relav+deci_eip)&0xFFFF);
	if (b == 0xE2)		sprintf(buf,"LOOP    %04Xh",(relav+deci_eip)&0xFFFF);

	AppendCallProcName(ireg_cs,(relav+deci_eip)&0xFFFF,buf);

    return 1;
}

int cpu_exec_CJ(BYTE b)
{
	BYTE b2;
	DWORD relav;
	int cnd;

	b2 = b&15;
	relav = (DWORD)CPUQueueFetch(); FlushCPUQueue();
	if (relav >= 0x80) relav |= 0xFFFFFF80;

/* JO */	if (b2 == 0)			cnd = ireg_flags & 0x800;
/* JNO */	if (b2 == 1)			cnd = !(ireg_flags & 0x800);
/* JB */	if (b2 == 2)			cnd = ireg_flags & 1;
/* JAE */	if (b2 == 3)			cnd = !(ireg_flags & 1);
/* JZ */	if (b2 == 4)			cnd = ireg_flags & 0x40;
/* JNZ */	if (b2 == 5)			cnd = !(ireg_flags & 0x40);
/* JBE */	if (b2 == 6)			cnd = (ireg_flags & 1) || (ireg_flags & 0x40);
/* JA */	if (b2 == 7)			cnd = !(ireg_flags & 1) && !(ireg_flags & 0x40);
/* JS */	if (b2 == 8)			cnd = ireg_flags & 0x80;
/* JNS */	if (b2 == 9)			cnd = !(ireg_flags & 0x80);
/* JP */	if (b2 == 10)			cnd = ireg_flags & 0x04;
/* JNP */	if (b2 == 11)			cnd = !(ireg_flags & 0x04);
/* JL */	if (b2 == 12)			cnd = (((ireg_flags & 0x80)?1:0) != ((ireg_flags & 0x800)?1:0));
/* JGE */	if (b2 == 13)			cnd = (((ireg_flags & 0x80)?1:0) == ((ireg_flags & 0x800)?1:0));
/* JLE */	if (b2 == 14)			cnd = ((ireg_flags & 0x40) || (((ireg_flags & 0x80)?1:0) != ((ireg_flags & 0x800)?1:0)));
/* JG */	if (b2 == 15)			cnd = (!(ireg_flags & 0x40) && (((ireg_flags & 0x80)?1:0) == ((ireg_flags & 0x800)?1:0)));

	if (cnd) ireg_eip = (ireg_eip + relav)&0xFFFF;

    return 1;
}

int cpu_dec_CJ(BYTE b,char *buf)
{
	BYTE b2;
	int relav;

	b2 = b&15;
	relav = (int)CPUDecoderQueueFetch();
	if (relav >= 0x80) relav |= 0xFFFFFF80;
	sprintf(buf,"%-7s %04Xh",conjmp[b2],(relav+deci_eip)&0xFFFF);

	AppendCallProcName(ireg_cs,(relav+deci_eip)&0xFFFF,buf);

    return 1;
}

int cpu_exec_JCXZ(BYTE b)
{
	DWORD relav;

	relav = (DWORD)CPUQueueFetch(); FlushCPUQueue();
	if (relav >= 0x80) relav |= 0xFFFFFF80;
	if (!(ireg_ecx&0xFFFF)) ireg_eip = (ireg_eip + relav)&0xFFFF;

    return 1;
}

int cpu_dec_JCXZ(BYTE b,char *buf)
{
	int relav;

	relav = (int)CPUDecoderQueueFetch();
	if (relav >= 0x80) relav |= 0xFFFFFF80;
	sprintf(buf,"JCXZ    %04Xh",(relav+deci_eip)&0xFFFF);

	AppendCallProcName(ireg_cs,(relav+deci_eip)&0xFFFF,buf);

    return 1;
}

/* I/O */
int cpu_exec_OUT(BYTE b)
{
	BYTE b2;

	if (b == 0xEE)				HardOut((WORD)(ireg_edx & 0xFFFF),(BYTE)(ireg_eax & 0xFF));
	if (b == 0xEF)				HardOut16((WORD)(ireg_edx & 0xFFFF),(WORD)(ireg_eax & 0xFFFF));
	if (b == 0xE6) {
		b2 = CPUQueueFetch();
		HardOut((WORD)b2,(BYTE)(ireg_eax&0xFF));
	}
	if (b == 0xE7) {
		b2 = CPUQueueFetch();
		HardOut16((WORD)b2,(WORD)(ireg_eax&0xFFFF));
	}

    return 1;
}

int cpu_dec_OUT(BYTE b,char *buf)
{
	BYTE b2;

	if (b == 0xEE)				sprintf(buf,"OUT     DX,AL");
	if (b == 0xEF)				sprintf(buf,"OUT     DX,AX");
	if (b == 0xE6) {
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"OUT     %02Xh,AL",b2);
	}
	if (b == 0xE7) {			// OUT imm,AX?
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"OUT     %02Xh,AX",b2);
	}

    return 1;
}

int cpu_exec_IN(BYTE b)
{
	BYTE b2;

	if (b == 0xEC) {
		ireg_eax &= ~0xFF;
		ireg_eax |= (DWORD)((BYTE)HardIn((WORD)ireg_edx));
	}
	if (b == 0xED) {
		ireg_eax &= ~0xFFFF;
		ireg_eax |= HardIn16((WORD)ireg_edx);
	}
	if (b == 0xE4) {
		b2 = CPUQueueFetch();
		ireg_eax &= ~0xFF;
		ireg_eax |= HardIn((WORD)b2);
	}
	if (b == 0xE5) {
		b2 = CPUQueueFetch();
		ireg_eax &= ~0xFFFF;
		ireg_eax |= HardIn16((WORD)b2);
	}

    return 1;
}

int cpu_dec_IN(BYTE b,char *buf)
{
	BYTE b2;

	if (b == 0xEC) {
		sprintf(buf,"IN      AL,DX");
	}
	if (b == 0xED) {
		sprintf(buf,"IN      AX,DX");
	}
	if (b == 0xE4) {
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"IN      AL,%02Xh",b2);
	}
	if (b == 0xE5) {
		b2 = CPUDecoderQueueFetch();
		sprintf(buf,"IN      AX,%02Xh",b2);
	}

    return 1;
}

/* address loading and calculating */

int cpu_exec_LDS(BYTE b)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	fl_d = fl_w = TRUE;

	ExecuteMODLDS(exec_LDS);

    return 1;
}

int cpu_dec_LDS(BYTE b,char *buf)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	fl_d = fl_w = TRUE;

	InterpretMODLIRM("LDS");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_LES(BYTE b)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	fl_d = fl_w = TRUE;

	ExecuteMODLDS(exec_LES);

    return 1;
}

int cpu_dec_LES(BYTE b,char *buf)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	fl_d = fl_w = TRUE;

	InterpretMODLIRM("LES");
	strcpy(buf,intcbuf);

    return 1;
}

int cpu_exec_LEA(BYTE b)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	fl_d = fl_w = TRUE;

	ExecuteMODLEA(exec_MOV);

    return 1;
}

int cpu_dec_LEA(BYTE b,char *buf)
{
    BYTE mrr;

/* fetch mod-reg-rm byte and decode */

    mrr = CPUDecoderQueueFetch();
    fl_mod = (int)((mrr>>6)&3);
    fl_reg = (int)((mrr>>3)&7);
    fl_rm  = (int)(mrr&7);
	fl_d = fl_w = TRUE;

	InterpretMODLEARM("LEA");
	strcpy(buf,intcbuf);

    return 1;
}

/* *******************************
   ** 8086 model function table **
   ******************************* */

MODELOPCODE mdl_cpu_8086[256] = {
/* 0x00 */
    {1,cpu_exec_ADD_MRR         ,cpu_dec_ADD_MRR        ,NULL},						/* 00h - AND */
    {1,cpu_exec_ADD_MRR         ,cpu_dec_ADD_MRR        ,NULL},						/* 01h - AND */
    {1,cpu_exec_ADD_MRR         ,cpu_dec_ADD_MRR        ,NULL},						/* 02h - AND */
    {1,cpu_exec_ADD_MRR         ,cpu_dec_ADD_MRR        ,NULL},						/* 03h - AND */
/* 0x04 */
    {1,cpu_exec_ADD_R_AX        ,cpu_dec_ADD_R_AX       ,NULL},						/* 04h - ADD AL,imm */
    {1,cpu_exec_ADD_R_AX        ,cpu_dec_ADD_R_AX       ,NULL},						/* 05h - ADD AX,imm */
    {1,cpu_exec_PUSHSEG         ,cpu_dec_PUSHSEG        ,NULL},                     /* 06h - PUSH ES */
    {1,cpu_exec_POPSEG          ,cpu_dec_POPSEG         ,NULL},                     /* 07h - POP ES */
/* 0x08 */
    {1,cpu_exec_OR_MRR          ,cpu_dec_OR_MRR         ,NULL},						/* 08h - OR mod-reg-rm */
    {1,cpu_exec_OR_MRR          ,cpu_dec_OR_MRR         ,NULL},						/* 09h - OR mod-reg-rm */
    {1,cpu_exec_OR_MRR          ,cpu_dec_OR_MRR         ,NULL},						/* 0Ah - OR mod-reg-rm */
    {1,cpu_exec_OR_MRR          ,cpu_dec_OR_MRR         ,NULL},						/* 0Bh - OR mod-reg-rm */
/* 0x0C */
    {1,cpu_exec_OR_R_AX         ,cpu_dec_OR_R_AX        ,NULL},						/* 0Ch - OR AL,imm */
    {1,cpu_exec_OR_R_AX         ,cpu_dec_OR_R_AX        ,NULL},						/* 0Dh - OR AX,imm */
    {1,cpu_exec_PUSHSEG         ,cpu_dec_PUSHSEG        ,NULL},                     /* 0Eh - PUSH CS */
//    {1,cpu_exec_POPSEG          ,cpu_dec_POPSEG         ,NULL},                     /* 0Fh - POP CS */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 0Fh - 286+ extension */
/* 0x10 */
    {1,cpu_exec_ADC_MRR         ,cpu_dec_ADC_MRR        ,NULL},						/* 10h - ADC */
    {1,cpu_exec_ADC_MRR         ,cpu_dec_ADC_MRR        ,NULL},						/* 11h - ADC */
    {1,cpu_exec_ADC_MRR         ,cpu_dec_ADC_MRR        ,NULL},						/* 12h - ADC */
    {1,cpu_exec_ADC_MRR         ,cpu_dec_ADC_MRR        ,NULL},						/* 13h - ADC */
/* 0x14 */
    {1,cpu_exec_ADC_R_AX        ,cpu_dec_ADC_R_AX       ,NULL},						/* 14h - ADC al,imm */
    {1,cpu_exec_ADC_R_AX        ,cpu_dec_ADC_R_AX       ,NULL},						/* 15h - ADC ax,imm */
    {1,cpu_exec_PUSHSEG         ,cpu_dec_PUSHSEG        ,NULL},                     /* 16h - PUSH SS */
    {1,cpu_exec_POPSEG          ,cpu_dec_POPSEG         ,NULL},                     /* 17h - POP SS */
/* 0x18 */
    {1,cpu_exec_SBB_MRR         ,cpu_dec_SBB_MRR        ,NULL},						/* 18h - SBB */
    {1,cpu_exec_SBB_MRR         ,cpu_dec_SBB_MRR        ,NULL},						/* 19h - SBB */
    {1,cpu_exec_SBB_MRR         ,cpu_dec_SBB_MRR        ,NULL},						/* 1Ah - SBB */
    {1,cpu_exec_SBB_MRR         ,cpu_dec_SBB_MRR        ,NULL},						/* 1Bh - SBB */
/* 0x1C */
    {1,cpu_exec_SBB_R_AX        ,cpu_dec_SBB_R_AX       ,NULL},						/* 1Ch - SBB AL,imm */
    {1,cpu_exec_SBB_R_AX        ,cpu_dec_SBB_R_AX       ,NULL},						/* 1Dh - SBB AX,imm */
    {1,cpu_exec_PUSHSEG         ,cpu_dec_PUSHSEG        ,NULL},                     /* 1Eh - PUSH DS */
    {1,cpu_exec_POPSEG          ,cpu_dec_POPSEG         ,NULL},                     /* 1Fh - POP DS */
/* 0x20 */
    {1,cpu_exec_AND_MRR         ,cpu_dec_AND_MRR        ,NULL},						/* 20h - AND mod-reg-rm */
    {1,cpu_exec_AND_MRR         ,cpu_dec_AND_MRR        ,NULL},						/* 21h - AND mod-reg-rm */
    {1,cpu_exec_AND_MRR         ,cpu_dec_AND_MRR        ,NULL},						/* 22h - AND mod-reg-rm */
    {1,cpu_exec_AND_MRR         ,cpu_dec_AND_MRR        ,NULL},						/* 23h - AND mod-reg-rm */
/* 0x24 */
    {1,cpu_exec_AND_R_AX        ,cpu_dec_AND_R_AX       ,NULL},						/* 24h - AND AL,imm */
    {1,cpu_exec_AND_R_AX        ,cpu_dec_AND_R_AX       ,NULL},						/* 25h - AND AX,imm */
    {0,cpu_exec_ES_PR           ,cpu_dec_ES_PR          ,NULL},                     /* 26h - ES: override */
    {1,cpu_exec_DAA             ,cpu_dec_DAA            ,NULL},						/* 27h - DAA */
/* 0x28 */
    {1,cpu_exec_SUB_MRR         ,cpu_dec_SUB_MRR        ,NULL},						/* 28h - SUB */
    {1,cpu_exec_SUB_MRR         ,cpu_dec_SUB_MRR        ,NULL},						/* 29h - SUB */
    {1,cpu_exec_SUB_MRR         ,cpu_dec_SUB_MRR        ,NULL},						/* 2Ah - SUB */
    {1,cpu_exec_SUB_MRR         ,cpu_dec_SUB_MRR        ,NULL},						/* 2Bh - SUB */
/* 0x2C */
    {1,cpu_exec_SUB_R_AX        ,cpu_dec_SUB_R_AX       ,NULL},						/* 2Ch - SUB AL,imm */
    {1,cpu_exec_SUB_R_AX        ,cpu_dec_SUB_R_AX       ,NULL},						/* 2Dh - SUB AX,imm */
    {0,cpu_exec_CS_PR           ,cpu_dec_CS_PR          ,NULL},                     /* 2Eh - CS: override */
    {1,cpu_exec_DAS             ,cpu_dec_DAS            ,NULL},						/* 2Fh - DAS */
/* 0x30 */
    {1,cpu_exec_XOR_MRR         ,cpu_dec_XOR_MRR        ,NULL},                     /* 30h - XOR (mod-reg-rm) BYTE */
    {1,cpu_exec_XOR_MRR         ,cpu_dec_XOR_MRR        ,NULL},                     /* 31h - XOR (mod-reg-rm) WORD */
    {1,cpu_exec_XOR_MRR         ,cpu_dec_XOR_MRR        ,NULL},                     /* 32h - XOR (mod-reg-rm) BYTE, opp. direction */
    {1,cpu_exec_XOR_MRR         ,cpu_dec_XOR_MRR        ,NULL},                     /* 33h - XOR (mod-reg-rm) WORD, opp. direction */
/* 0x34 */
    {1,cpu_exec_XOR_R_AX        ,cpu_dec_XOR_R_AX       ,NULL},						/* 34h - XOR AL,imm */
    {1,cpu_exec_XOR_R_AX        ,cpu_dec_XOR_R_AX       ,NULL},						/* 35h - XOR AX,imm */
    {0,cpu_exec_SS_PR           ,cpu_dec_SS_PR          ,NULL},                     /* 36h - SS: override */
    {1,cpu_exec_AAA             ,cpu_dec_AAA            ,NULL},						/* 37h - AAA */
/* 0x38 */
    {1,cpu_exec_CMP             ,cpu_dec_CMP            ,NULL},						/* 38h - CMP imm,AL */
    {1,cpu_exec_CMP             ,cpu_dec_CMP            ,NULL},						/* 39h - CMP imm,AX */
    {1,cpu_exec_CMP             ,cpu_dec_CMP            ,NULL},						/* 3Ah - CMP AL,imm */
    {1,cpu_exec_CMP             ,cpu_dec_CMP            ,NULL},						/* 3Bh - CMP AX,imm */
/* 0x3C */
    {1,cpu_exec_CMP_R_AX        ,cpu_dec_CMP_R_AX       ,NULL},						/* 3Ch - CMP al,imm */
    {1,cpu_exec_CMP_R_AX        ,cpu_dec_CMP_R_AX       ,NULL},						/* 3Dh - CMP ax,imm */
    {0,cpu_exec_DS_PR           ,cpu_dec_DS_PR          ,NULL},                     /* 3Eh - DS: override */
    {1,cpu_exec_AAS             ,cpu_dec_AAS            ,NULL},						/* 3Fh - AAS */
/* 0x40 */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 40h - INC */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 41h - INC */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 42h - INC */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 43h - INC */
/* 0x4C */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 44h - INC */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 45h - INC */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 46h - INC */
    {1,cpu_exec_INC_REG_IR      ,cpu_dec_INC_REG_IR     ,NULL},						/* 47h - INC */
/* 0x48 */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 48h - DEC */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 49h - DEC */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 4Ah - DEC */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 4Bh - DEC */
/* 0x4C */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 4Ch - DEC */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 4Dh - DEC */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 4Eh - DEC */
    {1,cpu_exec_DEC_REG_IR      ,cpu_dec_DEC_REG_IR     ,NULL},						/* 4Fh - DEC */
/* 0x50 */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 50h - PUSH */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 51h - PUSH */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 52h - PUSH */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 53h - PUSH */
/* 0x54 */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 54h - PUSH */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 55h - PUSH */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 56h - PUSH */
    {1,cpu_exec_PUSH_REG_IR     ,cpu_dec_PUSH_REG_IR    ,NULL},						/* 57h - PUSH */
/* 0x58 */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 58h - POP */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 59h - POP */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 5Ah - POP */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 5Bh - POP */
/* 0x5C */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 5Ch - POP */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 5Dh - POP */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 5Eh - POP */
    {1,cpu_exec_POP_REG_IR      ,cpu_dec_POP_REG_IR     ,NULL},						/* 5Fh - POP */
/* 0x60 */
    {1,cpu_exec_PUSHA           ,cpu_dec_PUSHA          ,NULL},						/* 60h - PUSHA */
    {1,cpu_exec_POPA            ,cpu_dec_POPA           ,NULL},						/* 61h - POPA */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 62h - BOUND */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 63h - ARPL */
/* 0x64 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 64h - FS: prefix */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 65h - GS: prefix */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 66h - 32-bit data extend */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 67h - code mod/reg/rm extend */
/* 0x68 */
    {1,cpu_exec_PUSHWORD_IMM    ,cpu_dec_PUSHWORD_IMM   ,NULL},						/* 68h - PUSH immediate WORD */
    {1,cpu_exec_IMUL_386        ,cpu_dec_IMUL_386       ,NULL},						/* 69h - IMUL [dest],[src],multiplier(W) */
    {1,cpu_exec_PUSHWORD_IMM    ,cpu_dec_PUSHWORD_IMM   ,NULL},						/* 6Ah - PUSH immediate byte sign-extended to WORD */
    {1,cpu_exec_IMUL_386        ,cpu_dec_IMUL_386       ,NULL},						/* 6Bh - IMUL [dest],[src],multiplier(B) */
/* 0x6C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 6Ch - INSB */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 6Dh - INSW */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 6Eh - OUTSB */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* 6Fh - OUTSW */
/* 0x70 */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 70h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 71h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 72h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 73h - J(conditional) */
/* 0x74 */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 74h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 75h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 76h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 77h - J(conditional) */
/* 0x78 */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 78h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 79h - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 7Ah - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 7Bh - J(conditional) */
/* 0x7C */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 7Ch - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 7Dh - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 7Eh - J(conditional) */
    {1,cpu_exec_CJ              ,cpu_dec_CJ             ,NULL},						/* 7Fh - J(conditional) */
/* 0x80 */
    {1,cpu_exec_batch_80        ,cpu_dec_batch_80       ,NULL},						/* 80h - BATCH 80h (ADD OR ADC SBB AND SUB XOR CMP) */
    {1,cpu_exec_batch_80        ,cpu_dec_batch_80       ,NULL},						/* 81h - BATCH 80h (ADD OR ADC SBB AND SUB XOR CMP) */
    {1,cpu_exec_batch_80        ,cpu_dec_batch_80       ,NULL},						/* 82h - BATCH 80h (ADD OR ADC SBB AND SUB XOR CMP) */
    {1,cpu_exec_batch_80        ,cpu_dec_batch_80       ,NULL},						/* 83h - BATCH 80h (ADD OR ADC SBB AND SUB XOR CMP) */
/* 0x84 */
    {1,cpu_exec_TEST            ,cpu_dec_TEST           ,NULL},						/* 84h - TEST */
    {1,cpu_exec_TEST            ,cpu_dec_TEST           ,NULL},						/* 85h - TEST */
    {1,cpu_exec_XCHG            ,cpu_dec_XCHG           ,NULL},						/* 86h - XCHG */
    {1,cpu_exec_XCHG            ,cpu_dec_XCHG           ,NULL},						/* 87h - XCHG */
/* 0x88 */
    {1,cpu_exec_MOV_MRR         ,cpu_dec_MOV_MRR        ,NULL},                     /* 88h - MOV mod-reg-rm */
    {1,cpu_exec_MOV_MRR         ,cpu_dec_MOV_MRR        ,NULL},                     /* 89h - MOV mod-reg-rm */
    {1,cpu_exec_MOV_MRR         ,cpu_dec_MOV_MRR        ,NULL},                     /* 8Ah - MOV mod-reg-rm */
    {1,cpu_exec_MOV_MRR         ,cpu_dec_MOV_MRR        ,NULL},                     /* 8Bh - MOV mod-reg-rm */
/* 0x8C */
    {1,cpu_exec_MOV_MRRSEG      ,cpu_dec_MOV_MRRSEG     ,NULL},						/* 8Ch - MOV mod-reg-rm, segreg */
    {1,cpu_exec_LEA             ,cpu_dec_LEA            ,NULL},						/* 8Dh - LEA mod-reg-rm */
    {1,cpu_exec_MOV_MRRSEG      ,cpu_dec_MOV_MRRSEG     ,NULL},						/* 8Eh - MOV segreg, mod-reg-rm */
    {1,cpu_exec_POP_MEM_286     ,cpu_dec_POP_MEM_286    ,NULL},						/* 8Fh - POP MEM */
/* 0x90 */
    {1,cpu_exec_NOP             ,cpu_dec_NOP            ,NULL},						/* 90h - NOP */
    {1,cpu_exec_XCHG_AX_REG     ,cpu_dec_XCHG_AX_REG    ,NULL},						/* 91h - XCHG AX,imm */
    {1,cpu_exec_XCHG_AX_REG     ,cpu_dec_XCHG_AX_REG    ,NULL},						/* 92h - XCHG AX,imm */
    {1,cpu_exec_XCHG_AX_REG     ,cpu_dec_XCHG_AX_REG    ,NULL},						/* 93h - XCHG AX,imm */
/* 0x94 */
    {1,cpu_exec_XCHG_AX_REG     ,cpu_dec_XCHG_AX_REG    ,NULL},						/* 94h - XCHG AX,imm */
    {1,cpu_exec_XCHG_AX_REG     ,cpu_dec_XCHG_AX_REG    ,NULL},						/* 95h - XCHG AX,imm */
    {1,cpu_exec_XCHG_AX_REG     ,cpu_dec_XCHG_AX_REG    ,NULL},						/* 96h - XCHG AX,imm */
    {1,cpu_exec_XCHG_AX_REG     ,cpu_dec_XCHG_AX_REG    ,NULL},						/* 97h - XCHG AX,imm */
/* 0x98 */
    {1,cpu_exec_CBW             ,cpu_dec_CBW            ,NULL},                     /* 98h - CBW */
    {1,cpu_exec_CWD             ,cpu_dec_CWD			,NULL},						/* 99h - CWD */
    {1,cpu_exec_CALLFAR         ,cpu_dec_CALLFAR        ,NULL},						/* 9Ah - CALL (intrasegment) */
    {1,cpu_exec_WAIT            ,cpu_dec_WAIT           ,NULL},						/* 9Bh - WAIT */
/* 0x9C */
    {1,cpu_exec_PUSHF           ,cpu_dec_PUSHF          ,NULL},						/* 9Ch - PUSHF */
    {1,cpu_exec_POPF            ,cpu_dec_POPF           ,NULL},						/* 9Dh - POPF */
    {1,cpu_exec_SAHF            ,cpu_dec_SAHF           ,NULL},						/* 9Eh - SAHF */
    {1,cpu_exec_LAHF            ,cpu_dec_LAHF           ,NULL},						/* 9Fh - LAHF */
/* 0xA0 */
    {1,cpu_exec_MOV_R_AX        ,cpu_dec_MOV_R_AX       ,NULL},						/* A0h - MOV AL,[imm] */
    {1,cpu_exec_MOV_R_AX        ,cpu_dec_MOV_R_AX       ,NULL},						/* A1h - MOV AX,[imm] */
    {1,cpu_exec_MOV_R_AX        ,cpu_dec_MOV_R_AX       ,NULL},						/* A2h - MOV [imm],AL */
    {1,cpu_exec_MOV_R_AX        ,cpu_dec_MOV_R_AX       ,NULL},						/* A3h - MOV [imm],AX */
/* 0xA4 */
    {1,cpu_exec_MOVS            ,cpu_dec_MOVS           ,NULL},                     /* A4h - MOVSB */
    {1,cpu_exec_MOVS            ,cpu_dec_MOVS           ,NULL},                     /* A5h - MOVSW */
    {1,cpu_exec_CMPS            ,cpu_dec_CMPS           ,NULL},						/* A6h - CMPSB */
    {1,cpu_exec_CMPS            ,cpu_dec_CMPS           ,NULL},						/* A7h - CMPSW */
/* 0xA8 */
    {1,cpu_exec_TEST_R_AX       ,cpu_dec_TEST_R_AX      ,NULL},						/* A8h - TEST AL,imm */
    {1,cpu_exec_TEST_R_AX       ,cpu_dec_TEST_R_AX      ,NULL},						/* A8h - TEST AX,imm */
    {1,cpu_exec_STOS            ,cpu_dec_STOS           ,NULL},                     /* AAh - STOSB */
    {1,cpu_exec_STOS            ,cpu_dec_STOS           ,NULL},                     /* ABh - STOSW */
/* 0xAC */
    {1,cpu_exec_LODS            ,cpu_dec_LODS           ,NULL},                     /* ACh - LODSB */
    {1,cpu_exec_LODS            ,cpu_dec_LODS           ,NULL},                     /* ADh - LODSW */
    {1,cpu_exec_SCAS            ,cpu_dec_SCAS           ,NULL},						/* AEh - SCASB */
    {1,cpu_exec_SCAS            ,cpu_dec_SCAS           ,NULL},						/* AFh - SCASW */
/* 0xB0 */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B0h - MOV byte reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B1h - MOV byte reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B2h - MOV byte reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B3h - MOV byte reg, imm */
/* 0xB4 */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B4h - MOV byte reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B5h - MOV byte reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B6h - MOV byte reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B7h - MOV byte reg, imm */
/* 0xB8 */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B8h - MOV word reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* B9h - MOV word reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* BAh - MOV word reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* BBh - MOV word reg, imm */
/* 0xBC */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* BCh - MOV word reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* BDh - MOV word reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* BEh - MOV word reg, imm */
    {1,cpu_exec_MOV_BOREGIMM    ,cpu_dec_MOV_BOREGIMM   ,NULL},						/* BFh - MOV word reg, imm */
/* 0xC0 */
    {1,cpu_exec_batch_C0        ,cpu_dec_batch_C0       ,NULL},						/* C0h - batch C0h */
    {1,cpu_exec_batch_C0        ,cpu_dec_batch_C0       ,NULL},						/* C1h - batch C0h */
    {1,cpu_exec_RET             ,cpu_dec_RET            ,NULL},						/* C2h - RET and pop N bytes */
    {1,cpu_exec_RET             ,cpu_dec_RET            ,NULL},						/* C3h - RET */
/* 0xC4 */
    {1,cpu_exec_LES             ,cpu_dec_LES            ,NULL},						/* C4h - LES */
    {1,cpu_exec_LDS             ,cpu_dec_LDS            ,NULL},						/* C5h - LDS */
    {1,cpu_exec_MOV_MRR_C6      ,cpu_dec_MOV_MRR_C6     ,NULL},						/* C6h - MOV mod-reg-rm */
    {1,cpu_exec_MOV_MRR_C6      ,cpu_dec_MOV_MRR_C6     ,NULL},						/* C7h - MOV mod-reg-rm */
/* 0xC8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* C8h - ENTER */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},						/* C9h - LEAVE */
    {1,cpu_exec_RETF            ,cpu_dec_RETF           ,NULL},						/* CAh - RETF and pop N bytes */
    {1,cpu_exec_RETF            ,cpu_dec_RETF           ,NULL},						/* CBh - RETF */
/* 0xCC */
    {1,cpu_exec_INT             ,cpu_dec_INT            ,NULL},						/* CCh - INT 3 */
    {1,cpu_exec_INT             ,cpu_dec_INT            ,NULL},						/* CDh - INT N */
    {1,cpu_exec_IRET            ,cpu_dec_IRET           ,NULL},						/* CEh - IRET and pop N bytes */
    {1,cpu_exec_IRET            ,cpu_dec_IRET           ,NULL},						/* CFh - IRET */
/* 0xD0 */
    {1,cpu_exec_batch_D0        ,cpu_dec_batch_D0       ,NULL},						/* D0h - batch D0h */
    {1,cpu_exec_batch_D0        ,cpu_dec_batch_D0       ,NULL},						/* D1h - batch D0h */
    {1,cpu_exec_batch_D2        ,cpu_dec_batch_D2       ,NULL},						/* D2h - batch D2h */
    {1,cpu_exec_batch_D2        ,cpu_dec_batch_D2       ,NULL},						/* D3h - batch D2h */
/* 0xD4 */
    {1,cpu_exec_AAM             ,cpu_dec_AAM            ,NULL},						/* D4h - AAM */
    {1,cpu_exec_AAD             ,cpu_dec_AAD            ,NULL},						/* D5h - AAD */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,cpu_exec_XLAT            ,cpu_dec_XLAT           ,NULL},						/* D7h - XLAT */
/* 0xD8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xDC */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xE0 */
    {1,cpu_exec_LOOP            ,cpu_dec_LOOP           ,NULL},						/* E0h - LOOPNZ */
    {1,cpu_exec_LOOP            ,cpu_dec_LOOP           ,NULL},						/* E1h - LOOPZ */
    {1,cpu_exec_LOOP            ,cpu_dec_LOOP           ,NULL},						/* E2h - LOOP */
    {1,cpu_exec_JCXZ            ,cpu_dec_JCXZ           ,NULL},						/* E3h - JCXZ */
/* 0xE4 */
    {1,cpu_exec_IN              ,cpu_dec_IN             ,NULL},						/* E4h - IN AL,DX */
    {1,cpu_exec_IN              ,cpu_dec_IN             ,NULL},						/* E5h - IN AX,DX */
    {1,cpu_exec_OUT             ,cpu_dec_OUT            ,NULL},						/* E6h - OUT DX,AX */
    {1,cpu_exec_OUT             ,cpu_dec_OUT            ,NULL},						/* E7h - OUT DX,AX */
/* 0xE8 */
    {1,cpu_exec_CALLNEAR        ,cpu_dec_CALLNEAR       ,NULL},                     /* E8h - CALL (near, relative) */
    {1,cpu_exec_JMPNEAR         ,cpu_dec_JMPNEAR        ,NULL},                     /* E9h - JMP (near, relative) */
    {1,cpu_exec_JMPFAR          ,cpu_dec_JMPFAR         ,NULL},						/* EAh - JMP (intrasegment) */
    {1,cpu_exec_JMPSHORT        ,cpu_dec_JMPSHORT       ,NULL},						/* EBh - JMP (short, relative) */
/* 0xEC */
    {1,cpu_exec_IN              ,cpu_dec_IN             ,NULL},						/* ECh - IN AL,DX */
    {1,cpu_exec_IN              ,cpu_dec_IN             ,NULL},						/* EDh - IN AX,DX */
    {1,cpu_exec_OUT             ,cpu_dec_OUT            ,NULL},						/* EEh - OUT DX,AL */
    {1,cpu_exec_OUT             ,cpu_dec_OUT            ,NULL},						/* EFh - OUT DX,AX */
/* 0xF0 */
    {0,cpu_exec_LOCK_PR         ,cpu_dec_LOCK_PR        ,NULL},                     /* F0h - LOCK */
    {1,cpu_exec_mystery_F1      ,cpu_dec_mystery_F1     ,NULL},						/* F1h - some mysterious instruction */
    {0,cpu_exec_REPNZ           ,cpu_dec_REPNZ          ,mdl_cpu_8086_REPL},        /* F2h - REPNE */
    {0,cpu_exec_REPZ            ,cpu_dec_REPZ           ,mdl_cpu_8086_REPL},        /* F3h - REP */
/* 0xF4 */
    {1,cpu_exec_HLT             ,cpu_dec_HLT            ,NULL},						/* F4h - HLT */
    {1,cpu_exec_CMC             ,cpu_dec_CMC            ,NULL},                     /* F5h - CMC */
    {1,cpu_exec_batch_F6        ,cpu_dec_batch_F6       ,NULL},                     /* F6h - BATCH F6h */
    {1,cpu_exec_batch_F6        ,cpu_dec_batch_F6       ,NULL},						/* F7h - BATCH F6h */
/* 0xF8 */
    {1,cpu_exec_CLC             ,cpu_dec_CLC            ,NULL},                     /* F8h - CLC */
    {1,cpu_exec_STC             ,cpu_dec_STC            ,NULL},						/* F9h - STC */
    {1,cpu_exec_CLI             ,cpu_dec_CLI            ,NULL},                     /* FAh - CLI */
    {1,cpu_exec_STI             ,cpu_dec_STI            ,NULL},                     /* FBh - STI */
/* 0xFC */
    {1,cpu_exec_CLD             ,cpu_dec_CLD            ,NULL},                     /* FCh - CLD */
    {1,cpu_exec_STD             ,cpu_dec_STD            ,NULL},						/* FDh - STD */
    {1,cpu_exec_batch_FE        ,cpu_dec_batch_FE       ,NULL},						/* FEh - BATCH FEh */
    {1,cpu_exec_batch_FE        ,cpu_dec_batch_FE       ,NULL},						/* FFh - BATCH FEh */
};

MODELOPCODE mdl_cpu_8086_REPL[256] = {
/* 0x00 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x04 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x08 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x0C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x10 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x14 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x18 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x1C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x20 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x24 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {0,cpu_exec_ES_PR           ,cpu_dec_ES_PR          ,NULL},                     /* 26h - ES: override */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x28 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x2C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {0,cpu_exec_CS_PR           ,cpu_dec_CS_PR          ,NULL},                     /* 2Eh - CS: override */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x30 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x34 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {0,cpu_exec_SS_PR           ,cpu_dec_SS_PR          ,NULL},                     /* 36h - SS: override */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x38 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x3C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {0,cpu_exec_DS_PR           ,cpu_dec_DS_PR          ,NULL},                     /* 3Eh - DS: override */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x40 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x44 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x48 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x4C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x50 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x54 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x58 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x5C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x60 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x64 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x68 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x6C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x70 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x74 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x78 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x7C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x80 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x84 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x88 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x8C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x90 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x94 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x98 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0x9C */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xA0 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xA4 */
    {1,cpu_exec_MOVS            ,cpu_dec_MOVS           ,NULL},                     /* A4h - MOVSB */
    {1,cpu_exec_MOVS            ,cpu_dec_MOVS           ,NULL},                     /* A5h - MOVSW */
    {1,cpu_exec_CMPS            ,cpu_dec_CMPS           ,NULL},						/* A6h - CMPSB */
    {1,cpu_exec_CMPS            ,cpu_dec_CMPS           ,NULL},						/* A7h - CMPSW */
/* 0xA8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,cpu_exec_STOS            ,cpu_dec_STOS           ,NULL},                     /* AAh - STOSB */
    {1,cpu_exec_STOS            ,cpu_dec_STOS           ,NULL},                     /* ABh - STOSW */
/* 0xAC */
    {1,cpu_exec_LODS            ,cpu_dec_LODS           ,NULL},                     /* ACh - LODSB */
    {1,cpu_exec_LODS            ,cpu_dec_LODS           ,NULL},                     /* ADh - LODSW */
    {1,cpu_exec_SCAS            ,cpu_dec_SCAS           ,NULL},                     /* AEh - SCASB */
    {1,cpu_exec_SCAS            ,cpu_dec_SCAS           ,NULL},                     /* AFh - SCASW */
/* 0xB0 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xB4 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xB8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xBC */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xC0 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xC4 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xC8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xCC */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xD0 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xD4 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xD8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xDC */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xE0 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xE4 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xE8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xEC */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xF0 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xF4 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xF8 */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
/* 0xFC */
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
    {1,mdl_exec_default         ,mdl_dec_default        ,NULL},
};
