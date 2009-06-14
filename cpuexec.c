/* the executioneer (the part that loads and executes the instructions */

#include "global.h"
#include "resource.h"
#include <stdio.h>
#include <math.h>
#include "fdsim.h"
#include "hdsim.h"
#include "cputail.h"
#include "cpustkw.h"
#include "bios.h"
#include "flagops.h"
#include "cpumodel.h"
#include "cpuqueue.h"
#include "cpudec.h"
#include "cpuexec.h"
#include "stackops.h"
#include "direct86.h"
#include "execops.h"
#include "execrm.h"
#include "brkpts.h"
#include "addrbus.h"
#include "hardware.h"
#include "mother.h"
#include "naming.h"

#include "8086.h"

extern int do_not_log_sp;
extern int exec_add_or_sub_sp;

char *ex_cpu_models[] =		{"8086/8088","80186","80286","80386","80486","80586/Pentium",NULL};
char *ex_cpu_wcachesiz[] =	{"Disabled","1KB","2KB","4KB","8KB","16KB","32KB","64KB","128KB","256KB","512KB","1024MB","2048MB","4096MB",NULL};
int							exec_cycle=0;
BOOL						code_freerun=FALSE,code_showallsteps=TRUE;
BOOL						exec_ROR_o=FALSE;
int							exec_RC_carry;
int							code_showstepcnt=0;
int							code_cpuspeed=1;
int							exec_show=0,exec_step=0;
int							cpu_revision=0;
BOOL						ExecuteBusy=FALSE;
BOOL						edb66=0,edb67=0;
int							exec_imm1;
int							exec_bit;
BOOL						exec_buslock=FALSE;
BOOL						pf_8088;
int							exec_buslockcnt=0;
DWORD						rep_loopaddr_cs,rep_loopaddr_ip;
BOOL						REPLoop=TRUE;
BOOL						REPZflag=0;
BOOL                        segov;
int                         procREP;
DWORD                       oireg_eip;
/* registers */
DWORD						ireg_eip=0;
DWORD						ireg_eax=0;
DWORD						ireg_ebx=0;
DWORD						ireg_ecx=0;
DWORD						ireg_edx=0;
DWORD						ireg_esi=0;
DWORD						ireg_edi=0;
DWORD						ireg_esp=0;
DWORD						ireg_ebp=0;
DWORD						ireg_cs=0;
DWORD						ireg_ds=0;
DWORD						ireg_es=0;
DWORD						ireg_fs=0;
DWORD						ireg_gs=0;
DWORD						ireg_ss=0;
DWORD						ireg_cr0=0;
DWORD						ireg_flags=0;
LONG						ireg_idtr=0;
LONG						ireg_gdtr=0;
WORD						ireg_ldtr=0;
DWORD*						ireg_dataseg = &ireg_ds;
int							ireg_datasegreset=0;
/* FPU registers */
BYTE						fpu_87_state[94];
long double					fpu_87_stack[8];
/* interrupt "pin" */
int							interrupt_signal = -1;
int							nmi_interrupt_signal = 0;
/* set by emulator code because future revisions of the 8086 */
/* family have certain opcodes exempt from interruption */
int							opcode_denies_intproc = 0;

DWORD *regptr_dec32[] = {
	((DWORD*)&ireg_eax),
	((DWORD*)&ireg_ecx),
	((DWORD*)&ireg_edx),
	((DWORD*)&ireg_ebx),
	((DWORD*)&ireg_esp),
	((DWORD*)&ireg_ebp),
	((DWORD*)&ireg_esi),
	((DWORD*)&ireg_edi)};

WORD *regptr_dec[] = {
	((WORD*)&ireg_eax),
	((WORD*)&ireg_ecx),
	((WORD*)&ireg_edx),
	((WORD*)&ireg_ebx),
	((WORD*)&ireg_esp),
	((WORD*)&ireg_ebp),
	((WORD*)&ireg_esi),
	((WORD*)&ireg_edi)};

BYTE *regptr_decbyte[] = {
	((BYTE*)&ireg_eax),
	((BYTE*)&ireg_ecx),
	((BYTE*)&ireg_edx),
	((BYTE*)&ireg_ebx),
	(((BYTE*)&ireg_eax) + 1),
	(((BYTE*)&ireg_ecx) + 1),
	(((BYTE*)&ireg_edx) + 1),
	(((BYTE*)&ireg_ebx) + 1)};

WORD *regptr_seg[] = {
	(WORD*)&ireg_es,
	(WORD*)&ireg_cs,
	(WORD*)&ireg_ss,
	(WORD*)&ireg_ds,
	(WORD*)&ireg_fs,
	(WORD*)&ireg_gs,
	NULL,
	NULL};

BOOL CALLBACK DlgCPUConfig(HWND hwnd,UINT iMessage,WPARAM wParam,LPARAM lParam) {
	int wID,x;
	HWND ctr;

	if (iMessage == WM_INITDIALOG) {
		ctr=GetDlgItem(hwnd,IDC_CPUSELECT_MODEL);
		for (x=0;ex_cpu_models[x] != NULL;x++)
			SendMessage(ctr,CB_ADDSTRING,0,(DWORD)ex_cpu_models[x]);

		SendMessage(ctr,CB_SETCURSEL,(WPARAM)cpu_revision,0);

		SetFocus(GetDlgItem(hwnd,IDOK));
	}
	else if (iMessage == WM_COMMAND) {
		wID = (int)wParam;
		if (wID == IDOK) {
			ctr=GetDlgItem(hwnd,IDC_CPUSELECT_MODEL);
			cpu_revision=SendMessage(ctr,CB_GETCURSEL,0,0);

			EndDialog(hwnd,0);
		}
		if (wID == IDCANCEL) {
			EndDialog(hwnd,0);
		}
	}
	return FALSE;
}

void ConfigureCPU()
{
	DialogBox(hInst,MAKEINTRESOURCE(IDD_CPUSELECT),hwndMain,DlgCPUConfig);
	SetFocus(hwndMain);
}

void InterruptCPU(int n)
{
	char buf[512];

	if (interrupt_signal >= 0) {
		sprintf(buf,"INTERRUPT WARNING: Interrupt %02X signalling before %02X can be processed",n,interrupt_signal);
		MessageBox(hwndMain,buf,"CPU Interrupt processing warning",MB_OK);
	}

	interrupt_signal = n;
}

int CanInterruptCPU()
{
	if (interrupt_signal >= 0 || nmi_interrupt_signal) return 0;

	return 1;
}

DWORD Execute()
{
    BYTE b;
	DWORD oip,ocs,oss,osp;
	DWORD begin_cs,begin_ip;
	BOOL pr=FALSE;
	int ow;

	if (!CPUpower || ExecuteBusy) return 0;

	AddPrevCPUAddr(ireg_cs,ireg_eip);

	ClearSeqMatches();
	ow=stack_watch_ops_w;
	oss=ireg_ss;
	osp=ireg_esp;
	opcode_denies_intproc=0;
	exec_show_breakpoint=0;
	begin_cs = ireg_cs;
	begin_ip = ireg_eip;
	ExecuteBusy=TRUE;
	REPLoop=FALSE;
	exec_buslockcnt=0;
	exec_buslock=FALSE;
	oireg_eip = ireg_eip;
	pr=FALSE;
	segov=FALSE;
	REPLoop=FALSE;
	REPZflag=0;
	rep_loopaddr_cs = 0;
	rep_loopaddr_ip = 0;
	procREP=FALSE;
	ireg_dataseg = &ireg_ds;
	ireg_datasegreset = 0;
	exec_add_or_sub_sp = 0;
	edb66=edb67=0;
	intcbuf[0] = 0;
	fl_prefix[0] = 0;
	pr=FALSE;
    b = 0;

	do_not_log_sp=1;

/*

   NOT SO FAST!!!! I need to emulate the BIOS routines, but I'm not about to
   write it all out and write out all the simulation code, for now at least...

   At certain addresses (inside the ROM area), we execute some important
   tasks. Then, when our task is done, the IRET at that particular memory
   location will return us to caller.

*/
	if (ireg_cs == 0xF000) {
/* are we at an address call for INT 00h (F000:EC00h) */
		if (ireg_eip == 0xEC00) {
			MessageBox(hwndMain,"Division by 0 exception!","",MB_OK);
			pr=TRUE;
			code_freerun=0;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 10h (F000:AB10h) */
		if (ireg_eip == 0xAB10) {
			BIOS_Int10();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 11h (F000:AB11h) */
		if (ireg_eip == 0xAB11) {
			BIOS_Int11();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 13h (F000:AB13h) */
        if (ireg_eip == 0xAB13) {
			BIOS_Int13();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 14h (F000:AB14h) */
		if (ireg_eip == 0xAB14) {
			BIOS_Int14();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 15h (F000:AB15h) */
		if (ireg_eip == 0xAB15) {
			BIOS_Int15();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 16h (F000:AB16h) */
		if (ireg_eip == 0xAB16) {
			BIOS_Int16();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 17h (F000:AB17h) */
		if (ireg_eip == 0xAB17) {
			BIOS_Int17();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
/* are we at an address call for INT 1Ah (F000:AB1Ah) */
		if (ireg_eip == 0xAB1A) {
			BIOS_Int1A();
			pr=TRUE;
			ireg_flags |= 0x200;
		}
	}

	do_not_log_sp=0;

/* if BIOS fakery did it's thing, say whatever it was is valid. The code below
   will ignore it and the executioneer will leave the instruction pointer the
   way the BIOS fakery set it. Otherwise, see if new table-based execution
   model recognizes instruction. */
   if (!pr) {
		pr=(BOOL)model_execute_opcode();
   }
   else {
		if (osp != ireg_esp) AddStackWatchEntry(SWT_IRET,ireg_cs,ireg_eip,begin_cs,begin_ip,oss,ireg_ss,osp,ireg_esp);
   }

/* if not recognized, jump back and fetch first opcode for the old code below.
   if the code is not recognized, set b to 0. pr is nonzero, so the old
   not-yet-ported-to-new-model code below can do it's work.

TODO: PORT OLD EMULATION CODE BELOW TO NEW TABLE-BASED MODEL
*/

#ifdef ZERO_ZERO_ZERO
/* "extended" opcodes (in other words, start with old "POP CS" instruction */
    if (b == 0x0F && cpu_revision >= 2 && !pr) {        /* anything other than 8086 only */
		b2 = membytefarptr(ireg_cs,ireg_eip++);

        if ((b2&0xF0) == 0x80 && !pr && cpu_revision >= 3) {    /* (386+) Jconditional? (near) */
			b2 &= 15;
			relav = (int)((signed short int)memwordfarptr(ireg_cs,ireg_eip));
			ireg_eip += 2;
/* JO? */
			if (b2 == 0) {
				if (ireg_flags & 0x800)		ireg_eip += relav;
			}
/* JNO? */
			if (b2 == 1) {
				if (!(ireg_flags & 0x800))	ireg_eip += relav;
			}
/* JB? */
			if (b2 == 2) {
				if (ireg_flags & 1)			ireg_eip += relav;
			}
/* JAE? */
			if (b2 == 3) {
				if (!(ireg_flags & 1))		ireg_eip += relav;
			}
/* JZ? */
			if (b2 == 4) {
				if (ireg_flags & 0x40)		ireg_eip += relav;
			}
/* JNZ? */
			if (b2 == 5) {
				if (!(ireg_flags & 0x40))	ireg_eip += relav;
            }
/* JBE? */
			if (b2 == 6) {
				if ((ireg_flags & 1) ||
					(ireg_flags & 0x40))	ireg_eip += relav;
            }
/* JA? */
			if (b2 == 7) {
				if (!(ireg_flags & 1))
					if (!(ireg_flags & 0x40))
						ireg_eip += relav;
			}
/* JS? */
			if (b2 == 8) {
				if (ireg_flags & 0x80)		ireg_eip += relav;
			}
/* JNS? */
			if (b2 == 9) {
				if (!(ireg_flags & 0x80))	ireg_eip += relav;
			}
/* JP? */
			if (b2 == 10) {
				if (ireg_flags & 0x04)		ireg_eip += relav;
			}
/* JNP? */
			if (b2 == 11) {
				if (!(ireg_flags & 0x04))	ireg_eip += relav;
			}
/* JL? */
			if (b2 == 12) {
				if (((ireg_flags & 0x80)?1:0) != ((ireg_flags & 0x800)?1:0)) ireg_eip += relav;
			}
/* JGE? */
			if (b2 == 13) {
				if (((ireg_flags & 0x80)?1:0) == ((ireg_flags & 0x800)?1:0)) ireg_eip += relav;
			}
/* JLE? */
			if (b2 == 14) {
				if ((ireg_flags & 0x40) ||
					(((ireg_flags & 0x80)?1:0) != ((ireg_flags & 0x800)?1:0))) ireg_eip += relav;
			}
/* JG? */
			if (b2 == 15) {
				if (!(ireg_flags & 0x40) &&
					(((ireg_flags & 0x80)?1:0) == ((ireg_flags & 0x800)?1:0))) ireg_eip += relav;
			}

			ireg_eip &= 0xFFFF;
			pr=TRUE;
		}
        if (b2 == 0x01 && cpu_revision >= 2 && !pr) {       /* (286+) LMSW r/m,reg */
			fl_w = TRUE;
			fl_d = FALSE;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);
			oip = ireg_eip;
			if (fl_reg == 0 && cpu_revision >= 3) ExecuteMODS1RMF(exec_SGDT);
			if (fl_reg == 1 && cpu_revision >= 3) ExecuteMODS1RMF(exec_SIDT);
			if (fl_reg == 2 && cpu_revision >= 3) ExecuteMODS1RMF(exec_LGDT);
			if (fl_reg == 3 && cpu_revision >= 3) ExecuteMODS1RMF(exec_LIDT);
			if (fl_reg == 4) ExecuteMODS1RM(exec_SMSW);
			if (fl_reg == 6) ExecuteMODS1RM(exec_LMSW);
			if (ireg_eip != oip)	pr=TRUE;
			else					{ pr=FALSE; ireg_eip--; }
		}
        if (b2 == 0x00 && cpu_revision >= 2 && !pr) {       /* (286+) SLDT */
			fl_w = TRUE;
			fl_d = FALSE;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);

			oip = ireg_eip;

            if (fl_reg == 0 && cpu_revision >= 3 && 0) ExecuteMODS1RMF(exec_SLDT); /* SLDT is not valid in real mode */

			if (ireg_eip != oip)	pr=TRUE;
			else					{ pr=FALSE; ireg_eip--; }
		}
        if ((b2&0xFE) == 0xB6 && cpu_revision >= 3 && !pr) {    /* (386+) MOVZX <word>,<byte>? */
			fl_d = 1;
			fl_w = b2&1;
			b = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b>>6)&3;
			fl_reg = (b>>3)&7;
			fl_rm  = (b&7);
			ExecuteMODMOVZX(exec_MOVZX);
			pr=TRUE;
		}
        if (b2 == 0xBC && cpu_revision >= 3 && !pr) {   /* (386+) BSF? */
			fl_s = FALSE;
			fl_w = TRUE;
			fl_fop = b2;
			fl_d = TRUE;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);
			ExecuteMODREGRM(exec_BSF386);
			pr=TRUE;
		}
        if (b2 == 0xBA && cpu_revision >= 3 && !pr) {   /* (386+) BTR? */
			fl_s = FALSE;
			fl_w = TRUE;
			fl_fop = b2;
			fl_d = TRUE;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);
			ExecuteMODREGRMBT(exec_BTR386);
			pr=TRUE;
		}
        if (b2 == 0xA4 && cpu_revision >= 3 && !pr) {   /* (386+) SHLD? */
			fl_s = FALSE;
			fl_w = TRUE;
			fl_fop = b2;
			fl_d = FALSE;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);
			ExecuteMODREGRMSHXD(exec_SHLD386);
			pr=TRUE;
		}
        if (b2 == 0xAC && cpu_revision >= 3 && !pr) {   /* (386+) SHRD? */
			fl_s = FALSE;
			fl_w = TRUE;
			fl_fop = b2;
			fl_d = FALSE;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);
			ExecuteMODREGRMSHXD(exec_SHRD386);
			pr=TRUE;
		}
        if ((b2&0xFE) == 0xC0 && !pr) { /* XADD? */
			fl_s = FALSE;
			fl_w = (b2&1);
			fl_fop = b2;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);
			ExecuteMODREGRMXCHG(exec_XADD);
			pr=TRUE;
		}
        if (b2 == 0xB2 && cpu_revision >= 3 && !pr) { /* (386+) LSS? */
			fl_d = fl_w = TRUE;
			b2 = membytefarptr(ireg_cs,ireg_eip++);
			fl_mod = (b2>>6)&3;
			fl_reg = (b2>>3)&7;
			fl_rm  = (b2&7);
			ExecuteMODLDS(exec_LSS);
			pr=TRUE;
		}
        if (b2 == 0xA1 && cpu_revision >= 3 && !pr) { /* (386+) POP FS */
			ireg_fs = (DWORD)PopWord();
			pr=TRUE;
		}

/* we've already got 0Fh, but we fetched another. The last thing we want is
   the code following us to take what we fetched and execute it as a valid
   instruction. So, signal illegal but recognized. */
		if (!pr) {
			pr=TRUE;
			illegal=TRUE;
		}
	}
#endif ZERO_ZERO_ZERO

	HandleSeqMatches();

	if (!pr) {
		char *msg = "An instruction opcode was not recognized or was " \
			"misinterpreted by the simulator.\n" \
			"\n" \
			"Click OK to continue execution and simulate an Invalid " \
			"Opcode exception\n\nClick Cancel to stop execution to look " \
			"at it.";

// call INT 06h (Invalid Opcode)
		debug_window_offset_cs = begin_cs;
		debug_window_offset_ip = begin_ip;
		ireg_eip = begin_ip;
		ireg_cs = begin_cs;
		ExecutionShow();
		MessageBeep(0xFFFFFFFF);

		if (MessageBox(hwndMain,msg,"Execution error",MB_OKCANCEL) == IDCANCEL) {
			code_freerun=FALSE;
		}
		else {
			SignalInterrupt16(0x06);
		}
	}

	if (exec_buslockcnt)			exec_buslockcnt--;
	else							exec_buslock=FALSE;

// impose flag behavior specific to CPU
	if (cpu_revision <= 1)			ireg_flags &= 0x0FFF;
	else if (cpu_revision <= 2)		ireg_flags |= 0x7000;

	if (exec_show_breakpoint) {
		debug_window_offset_cs = begin_cs;
		debug_window_offset_ip = begin_ip;
		ExecutionShow();
	}

	do_not_log_sp=1;

	if (interrupt_signal >= 0 && (ireg_flags & 0x200) && !opcode_denies_intproc) {
		oip = ireg_eip;
		ocs = ireg_cs;
		SignalInterrupt16(interrupt_signal);
		CheckTriggerHWINTBreakpoint(interrupt_signal);
		AddStackWatchEntryINT(SWT_HWCALLINT,ireg_cs,ireg_eip,ocs,oip,ireg_ss,ireg_ss,ireg_esp+6,ireg_esp,interrupt_signal);
		interrupt_signal = -1;
	}

	if (nmi_interrupt_signal) {
		oip = ireg_eip;
		ocs = ireg_cs;
		SignalInterrupt16(2);
		CheckTriggerHWINTBreakpoint(2);
		AddStackWatchEntryINT(SWT_HWCALLINT,ireg_cs,ireg_eip,ocs,oip,ireg_ss,ireg_ss,ireg_esp+6,ireg_esp,2);
		nmi_interrupt_signal = 0;
	}

	do_not_log_sp=0;

	ComputerCycle();
	ExecuteBusy=FALSE;
	exec_cycle++;

/* if nothing logged for CPU activity, log change of SP as complete disregard to stack */
	if (stack_watch_ops_w == ow) {
		if (oss != ireg_ss || osp != ireg_esp) {
			if (exec_add_or_sub_sp) {
				if (osp < ireg_esp)		AddStackWatchEntry(SWT_POPIGNORE,ireg_cs,ireg_eip,begin_cs,begin_ip,oss,ireg_ss,osp,ireg_esp);
				if (osp > ireg_esp)		AddStackWatchEntry(SWT_PUSHALLOC,ireg_cs,ireg_eip,begin_cs,begin_ip,oss,ireg_ss,osp,ireg_esp);
			}
			else {
				AddStackWatchEntry(SWT_DISREGARD,ireg_cs,ireg_eip,begin_cs,begin_ip,oss,ireg_ss,osp,ireg_esp);
			}
		}
	}

	return 0;
}

void ExecutioneerReset()
{
	set_cpu_model(mdl_cpu_8086);
	FlushCPUQueue();

	interrupt_signal = -1;
	opcode_denies_intproc = 0;
	nmi_interrupt_signal = 0;
	ireg_cs = 0xF000;
	ireg_ds = ireg_es = ireg_fs = ireg_gs = ireg_ss = 0;
	ireg_eip = 0xFFF0;
	ireg_eax = ireg_ebx = ireg_ecx = ireg_esi = ireg_edi = ireg_esp = ireg_ebp = 0;
	ireg_edx = 0;
	ireg_flags = 0x200;
	ireg_idtr = 0x0000000003FF;
	ireg_gdtr = 0x00000000FFFF;

	REPLoop = FALSE;
	exec_cycle = 0;
}

void ExecutionStep()
{
	Execute();
}

void ExecutionShow()
{
	debug_window_offset_cs = ireg_cs;
	debug_window_offset_ip = ireg_eip;
	RepaintSourceDump(FALSE);
}

void StepExecute()
{
	if (code_freerun) {
		ExecutionStep();
		if (code_showallsteps || code_showstepcnt >= code_cpuspeed) {
			ExecutionShow();
		}
		if (code_showstepcnt >= code_cpuspeed) {
			code_showstepcnt = 0;
		}
		else {
			code_showstepcnt++;
		}
	}
}

void InitExecutioneer()
{
	cpu_revision = GetPrivateProfileInt("CPU","revision",0,common_ini_file);
	code_cpuspeed = GetPrivateProfileInt("CPU","ins_per_update",0,common_ini_file);

	ExecutioneerReset();
	CPUpower=FALSE;
}

void CloseExecutioneer()
{
	char buf[512];

	sprintf(buf,"%d",cpu_revision);
	WritePrivateProfileString("CPU","revision",buf,common_ini_file);

	sprintf(buf,"%d",code_cpuspeed);
	WritePrivateProfileString("CPU","ins_per_update",buf,common_ini_file);

	ExecutioneerPowerOff();
}

void ExecutioneerPowerOn()
{
	CPUpower=TRUE;
}

void ExecutioneerPowerOff()
{
	ExecutioneerReset();
	CPUpower=FALSE;
}

int ExecutioneerIsPowered()
{
	return (int)CPUpower;
}
