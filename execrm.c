
#include "global.h"
#include "direct86.h"
#include "stackops.h"
#include "execrm.h"
#include "cpudec.h"
#include "cpuexec.h"
#include "cpuqueue.h"
#include "addrbus.h"

DWORD RMEAddress(int fl_rm,DWORD ofs)
{
	DWORD ea=0;

	if (fl_rm == 0)
		ea = (ireg_ebx+ireg_esi+ofs);
	if (fl_rm == 1)
		ea = (ireg_ebx+ireg_edi+ofs);
	if (fl_rm == 2)
		ea = (ireg_ebp+ireg_esi+ofs);
	if (fl_rm == 3)
		ea = (ireg_ebp+ireg_edi+ofs);
	if (fl_rm == 4)
		ea = (ireg_esi+ofs);
	if (fl_rm == 5)
		ea = (ireg_edi+ofs);
	if (fl_rm == 6)
		ea = (ireg_ebp+ofs);
	if (fl_rm == 7)
		ea = (ireg_ebx+ofs);

	return ea;
}

DWORD RMAddressbfar(int fl_rm,int ofs)
{
	DWORD b=0;
	int r_seg,r_off;

	if (fl_rm == 0) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ireg_esi+ofs; };
	if (fl_rm == 1) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ireg_edi+ofs; };
	if (fl_rm == 2) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebp+ireg_esi+ofs; };
	if (fl_rm == 3) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebp+ireg_edi+ofs; };
	if (fl_rm == 4) { r_seg = (int)*ireg_dataseg; r_off = ireg_esi+ofs; };
	if (fl_rm == 5) { r_seg = (int)*ireg_dataseg; r_off = ireg_edi+ofs; };
	if (fl_rm == 6) {
		if (ireg_datasegreset)	r_seg = (int)*ireg_dataseg;
		else					r_seg = (int)ireg_ss;

		r_off = ((int)ireg_ebp)+ofs;
	}
	if (fl_rm == 7) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ofs; };

// round stuff
	r_seg &= 0xFFFF;
	r_off &= 0xFFFF;

	b = (DWORD)membytefarptr((DWORD)r_seg,(DWORD)r_off);

	return b;
}

DWORD RMAddressbfar32(int fl_rm,int ofs)
{
	DWORD b=0;
	int r_seg,r_off;

	if (fl_rm == 0) { r_seg = (int)*ireg_dataseg; r_off = ireg_eax+ofs; };
	if (fl_rm == 1) { r_seg = (int)*ireg_dataseg; r_off = ireg_ecx+ofs; };
	if (fl_rm == 2) { r_seg = (int)*ireg_dataseg; r_off = ireg_edx+ofs; };
	if (fl_rm == 3) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ofs; };
	if (fl_rm == 4) {
		if (ireg_datasegreset)	r_seg = (int)*ireg_dataseg;
		else					r_seg = (int)ireg_ss;

		r_off = (int)ireg_esp+ofs;
	}
	if (fl_rm == 5) {
		if (ireg_datasegreset)	r_seg = (int)*ireg_dataseg;
		else					r_seg = (int)ireg_ss;

		r_off = (int)ireg_ebp+ofs;
	}
	if (fl_rm == 6) { r_seg = (int)*ireg_dataseg; r_off = ireg_esi+ofs; };
	if (fl_rm == 7) { r_seg = (int)*ireg_dataseg; r_off = ireg_edi+ofs; };

// round stuff
	r_seg &= 0xFFFF;

	b = (DWORD)membytefarptr((DWORD)r_seg,(DWORD)r_off);

	return b;
}

DWORD RMAddresswfar(int fl_rm,int ofs)
{
	DWORD w=0;

	w = RMAddressbfar(fl_rm,ofs);
	w |= RMAddressbfar(fl_rm,ofs+1)<<8;

	return w;
}

DWORD RMAddresswfar32(int fl_rm,int ofs)
{
	DWORD w=0;

	w = RMAddressbfar32(fl_rm,ofs);
	w |= RMAddressbfar32(fl_rm,ofs+1)<<8;

	return w;
}

DWORD RMAddressdfar(int fl_rm,int ofs)
{
	DWORD w=0;

	w = RMAddressbfar(fl_rm,ofs);
	w |= RMAddressbfar(fl_rm,ofs+1)<<8;
	w |= RMAddressbfar(fl_rm,ofs+2)<<16;
	w |= RMAddressbfar(fl_rm,ofs+3)<<24;

	return w;
}

DWORD RMAddressdfar32(int fl_rm,int ofs)
{
	DWORD w=0;

	w = RMAddressbfar32(fl_rm,ofs);
	w |= RMAddressbfar32(fl_rm,ofs+1)<<8;
	w |= RMAddressbfar32(fl_rm,ofs+2)<<16;
	w |= RMAddressbfar32(fl_rm,ofs+3)<<24;

	return w;
}

void RMAddressbfarw(int fl_rm,int ofs,DWORD d)
{
	int r_seg,r_off;

	if (fl_rm == 0) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ireg_esi+ofs; };
	if (fl_rm == 1) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ireg_edi+ofs; };
	if (fl_rm == 2) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebp+ireg_esi+ofs; };
	if (fl_rm == 3) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebp+ireg_edi+ofs; };
	if (fl_rm == 4) { r_seg = (int)*ireg_dataseg; r_off = ireg_esi+ofs; };
	if (fl_rm == 5) { r_seg = (int)*ireg_dataseg; r_off = ireg_edi+ofs; };
	if (fl_rm == 6) {
		if (ireg_datasegreset)	r_seg = (int)*ireg_dataseg;
		else					r_seg = (int)ireg_ss;

		r_off = (int)ireg_ebp+ofs;
	}
	if (fl_rm == 7) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ofs; };

// round stuff
	r_seg &= 0xFFFF;
	r_off &= 0xFFFF;

	writemembytefarptr(r_seg,r_off,(BYTE)d);
}

void RMAddressbfarw32(int fl_rm,int ofs,DWORD d)
{
	int r_seg,r_off;

	if (fl_rm == 0) { r_seg = (int)*ireg_dataseg; r_off = ireg_eax+ofs; };
	if (fl_rm == 1) { r_seg = (int)*ireg_dataseg; r_off = ireg_ecx+ofs; };
	if (fl_rm == 2) { r_seg = (int)*ireg_dataseg; r_off = ireg_edx+ofs; };
	if (fl_rm == 3) { r_seg = (int)*ireg_dataseg; r_off = ireg_ebx+ofs; };
	if (fl_rm == 4) {
		if (ireg_datasegreset)	r_seg = (int)*ireg_dataseg;
		else					r_seg = (int)ireg_ss;

		r_off = (int)ireg_esp+ofs;
	}
	if (fl_rm == 5) {
		if (ireg_datasegreset)	r_seg = (int)*ireg_dataseg;
		else					r_seg = (int)ireg_ss;

		r_off = (int)ireg_esp+ofs;
	}
	if (fl_rm == 6) { r_seg = (int)*ireg_dataseg; r_off = ireg_esi+ofs; };
	if (fl_rm == 7) { r_seg = (int)*ireg_dataseg; r_off = ireg_edi+ofs; };

// round stuff
	r_off &= 0xFFFF;

	writemembytefarptr(r_seg,r_off,(BYTE)d);
}

void RMAddresswfarw(int fl_rm,int ofs,DWORD w)
{
	RMAddressbfarw(fl_rm,ofs,w&255);
	RMAddressbfarw(fl_rm,ofs+1,(w>>8)&255);
}

void RMAddresswfarw32(int fl_rm,int ofs,DWORD w)
{
	RMAddressbfarw32(fl_rm,ofs,w&255);
	RMAddressbfarw32(fl_rm,ofs+1,(w>>8)&255);
}

void RMAddressdfarw(int fl_rm,int ofs,DWORD d)
{
	RMAddressbfarw(fl_rm,ofs,d&255);
	RMAddressbfarw(fl_rm,ofs+1,(d>>8)&255);
	RMAddressbfarw(fl_rm,ofs+2,(d>>16)&255);
	RMAddressbfarw(fl_rm,ofs+3,(d>>24)&255);
}

void RMAddressdfarw32(int fl_rm,int ofs,DWORD d)
{
	RMAddressbfarw32(fl_rm,ofs,d&255);
	RMAddressbfarw32(fl_rm,ofs+1,(d>>8)&255);
	RMAddressbfarw32(fl_rm,ofs+2,(d>>16)&255);
	RMAddressbfarw32(fl_rm,ofs+3,(d>>24)&255);
}

void ExecuteMODREGRM(void(*callfunc)(void*,void*,int))
{
	DWORD dw;
	WORD w,wa;
	BYTE b;
	int ol;

	if (1) {
		if (fl_mod == 0) {
			if (fl_rm == 6) {
				if (edb66) {
					wa = CPUQueueFetch();
					wa |= (CPUQueueFetch()<<8);

					dw = memdwordfarptr(*ireg_dataseg,wa);
					if (fl_d) {
						callfunc(&dw,regptr_dec32[fl_reg],4);
					}
					else {
						callfunc(regptr_dec32[fl_reg],&dw,4);
						writememdwordfarptr(*ireg_dataseg,wa,dw);
					}
				}
				else if (fl_w) {
					wa = CPUQueueFetch();
					wa |= (CPUQueueFetch()<<8);

					w = memwordfarptr(*ireg_dataseg,wa);
					if (fl_d) {
						callfunc(&w,regptr_dec[fl_reg],2);
					}
					else {
						callfunc(regptr_dec[fl_reg],&w,2);
						writememwordfarptr(*ireg_dataseg,wa,w);
					}
				}
				else {
					wa = CPUQueueFetch();
					wa |= (CPUQueueFetch()<<8);

					b = membytefarptr(*ireg_dataseg,wa);
					if (fl_d) {
						callfunc(&b,regptr_decbyte[fl_reg],1);
					}
					else {
						callfunc(regptr_decbyte[fl_reg],&b,1);
						writemembytefarptr(*ireg_dataseg,wa,b);
					}
				}
			}	
			else {
				if (edb66) {
					dw = RMAddressdfar(fl_rm,0);
					if (fl_d) {
						callfunc(&dw,regptr_dec32[fl_reg],4);
					}
					else {
						callfunc(regptr_dec32[fl_reg],&dw,4);
						RMAddressdfarw(fl_rm,0,dw);
					}
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar(fl_rm,0);
					if (fl_d) {
						callfunc(&w,regptr_dec[fl_reg],2);
					}
					else {
						callfunc(regptr_dec[fl_reg],&w,2);
						RMAddresswfarw(fl_rm,0,w);
					}
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,0);
					if (fl_d) {
						callfunc(&b,regptr_decbyte[fl_reg],1);
					}
					else {
						callfunc(regptr_decbyte[fl_reg],&b,1);
						RMAddressbfarw(fl_rm,0,b);
					}
				}
			}
		}
		if (fl_mod == 1) {
			ol = (int)((char)CPUQueueFetch());

			if (edb66) {
				dw = RMAddressdfar(fl_rm,ol);
				if (fl_d) {
					callfunc(&dw,regptr_dec32[fl_reg],4);
				}
				else {
					callfunc(regptr_dec32[fl_reg],&dw,4);
					RMAddressdfarw(fl_rm,ol,dw);
				}
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,ol);
				if (fl_d) {
					callfunc(&w,regptr_dec[fl_reg],2);
				}
				else {
					callfunc(regptr_dec[fl_reg],&w,2);
					RMAddresswfarw(fl_rm,ol,w);
				}
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,ol);
				if (fl_d) {
					callfunc(&b,regptr_decbyte[fl_reg],1);
				}
				else {
					callfunc(regptr_decbyte[fl_reg],&b,1);
					RMAddressbfarw(fl_rm,ol,b);
				}
			}
		}
		if (fl_mod == 2) {
			wa = CPUQueueFetch();
			wa |= (CPUQueueFetch()<<8);
			ol = (int)(wa);

			if (fl_d) {
				if (edb66) {
					dw = RMAddressdfar(fl_rm,ol);
					callfunc(&dw,regptr_dec32[fl_reg],4);
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar(fl_rm,ol);
					callfunc(&w,regptr_dec[fl_reg],2);
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,ol);
					callfunc(&b,regptr_decbyte[fl_reg],1);
				}
			}
			else {
				if (edb66) {
					dw = RMAddressdfar(fl_rm,ol);
					callfunc(regptr_dec32[fl_reg],&dw,4);
					RMAddressdfarw(fl_rm,ol,dw);
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar(fl_rm,ol);
					callfunc(regptr_dec[fl_reg],&w,2);
					RMAddresswfarw(fl_rm,ol,w);
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,ol);
					callfunc(regptr_decbyte[fl_reg],&b,1);
					RMAddressbfarw(fl_rm,ol,b);
				}
			}
		}
		if (fl_mod == 3) {
			if (edb66) {
				if (fl_d)
					callfunc(regptr_dec32[fl_rm],regptr_dec32[fl_reg],4);
				else
					callfunc(regptr_dec32[fl_reg],regptr_dec32[fl_rm],4);
			}
			else if (fl_w) {
				if (fl_d)
					callfunc(regptr_dec[fl_rm],regptr_dec[fl_reg],2);
				else
					callfunc(regptr_dec[fl_reg],regptr_dec[fl_rm],2);
			}
			else {
				if (fl_d)
					callfunc(regptr_decbyte[fl_rm],regptr_decbyte[fl_reg],1);
				else
					callfunc(regptr_decbyte[fl_reg],regptr_decbyte[fl_rm],1);
			}
		}
		edb66=0;
	}
}

void ExecuteMODREGRMBT(void(*callfunc)(void*,void*,int,int))
{
	DWORD dw;
	WORD w,wa;
	BYTE b,ba;
	int ol;

	if (1) {
		if (fl_mod == 0) {
			if (fl_rm == 6) {
				wa = CPUQueueFetch();
				wa |= (CPUQueueFetch()<<8);
				ba = CPUQueueFetch();

				if (edb66) {
					dw = memdwordfarptr(*ireg_dataseg,wa);
					exec_bit = ba;
					if (fl_d) {
						callfunc(&dw,regptr_dec32[fl_reg],4,exec_bit);
					}
					else {
						callfunc(regptr_dec32[fl_reg],&dw,4,exec_bit);
						writememdwordfarptr(*ireg_dataseg,wa,dw);
					}
				}
				else if (fl_w) {
					w = memwordfarptr(*ireg_dataseg,wa);
					exec_bit = ba;
					if (fl_d) {
						callfunc(&w,regptr_dec[fl_reg],2,exec_bit);
					}
					else {
						callfunc(regptr_dec[fl_reg],&w,2,exec_bit);
						writememwordfarptr(*ireg_dataseg,wa,w);
					}
				}
				else {
					b = membytefarptr(*ireg_dataseg,wa);
					exec_bit = ba;
					if (fl_d) {
						callfunc(&b,regptr_decbyte[fl_reg],1,exec_bit);
					}
					else {
						callfunc(regptr_decbyte[fl_reg],&b,1,exec_bit);
						writemembytefarptr(*ireg_dataseg,wa,b);
					}
				}
			}	
			else {
				ba = CPUQueueFetch();
				exec_bit = ba;
				if (edb66) {
					dw = RMAddressdfar(fl_rm,0);
					if (fl_d) {
						callfunc(&dw,regptr_dec32[fl_reg],4,exec_bit);
					}
					else {
						callfunc(regptr_dec32[fl_reg],&dw,4,exec_bit);
						RMAddressdfarw(fl_rm,0,dw);
					}
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar(fl_rm,0);
					if (fl_d) {
						callfunc(&w,regptr_dec[fl_reg],2,exec_bit);
					}
					else {
						callfunc(regptr_dec[fl_reg],&w,2,exec_bit);
						RMAddresswfarw(fl_rm,0,w);
					}
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,0);
					if (fl_d) {
						callfunc(&b,regptr_decbyte[fl_reg],1,exec_bit);
					}
					else {
						callfunc(regptr_decbyte[fl_reg],&b,1,exec_bit);
						RMAddressbfarw(fl_rm,0,b);
					}
				}
				ireg_eip++;
			}
		}
		if (fl_mod == 1) {
			ol = ((int)((char)CPUQueueFetch()));
			exec_bit = CPUQueueFetch();
			if (edb66) {
				dw = RMAddressdfar(fl_rm,ol);
				if (fl_d) {
					callfunc(&dw,regptr_dec32[fl_reg],4,exec_bit);
				}
				else {
					callfunc(regptr_dec32[fl_reg],&dw,4,exec_bit);
					RMAddressdfarw(fl_rm,ol,dw);
				}
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,ol);
				if (fl_d) {
					callfunc(&w,regptr_dec[fl_reg],2,exec_bit);
				}
				else {
					callfunc(regptr_dec[fl_reg],&w,2,exec_bit);
					RMAddresswfarw(fl_rm,ol,w);
				}
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,ol);
				if (fl_d) {
					callfunc(&b,regptr_decbyte[fl_reg],1,exec_bit);
				}
				else {
					callfunc(regptr_decbyte[fl_reg],&b,1,exec_bit);
					RMAddressbfarw(fl_rm,ol,b);
				}
			}
		}
		if (fl_mod == 2) {
			wa = CPUQueueFetch();
			wa |= (CPUQueueFetch()<<8);
			ol = (int)(wa);
			exec_bit = CPUQueueFetch();
			if (fl_d) {
				if (edb66) {
					dw = RMAddressdfar(fl_rm,ol);
					callfunc(&dw,regptr_dec32[fl_reg],4,exec_bit);
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar(fl_rm,ol);
					callfunc(&w,regptr_dec[fl_reg],2,exec_bit);
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,ol);
					callfunc(&b,regptr_decbyte[fl_reg],1,exec_bit);
				}
			}
			else {
				if (edb66) {
					dw = RMAddressdfar(fl_rm,ol);
					callfunc(regptr_dec32[fl_reg],&dw,4,exec_bit);
					RMAddressdfarw(fl_rm,ol,dw);
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar(fl_rm,ol);
					callfunc(regptr_dec[fl_reg],&w,2,exec_bit);
					RMAddresswfarw(fl_rm,ol,w);
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,ol);
					callfunc(regptr_decbyte[fl_reg],&b,1,exec_bit);
					RMAddressbfarw(fl_rm,ol,b);
				}
			}
		}
		if (fl_mod == 3) {
			exec_bit = CPUQueueFetch();
			if (edb66) {
				if (fl_d)
					callfunc(regptr_dec32[fl_rm],regptr_dec32[fl_reg],4,exec_bit);
				else
					callfunc(regptr_dec32[fl_reg],regptr_dec32[fl_rm],4,exec_bit);
			}
			else if (fl_w) {
				if (fl_d)
					callfunc(regptr_dec[fl_rm],regptr_dec[fl_reg],2,exec_bit);
				else
					callfunc(regptr_dec[fl_reg],regptr_dec[fl_rm],2,exec_bit);
			}
			else {
				if (fl_d)
					callfunc(regptr_decbyte[fl_rm],regptr_decbyte[fl_reg],1,exec_bit);
				else
					callfunc(regptr_decbyte[fl_reg],regptr_decbyte[fl_rm],1,exec_bit);
			}
		}
		edb66=0;
	}
}

void ExecuteMODMOVZX(void(*callfunc)(void*,void*,int,int))
{
	DWORD src;
	WORD wa;

	if (fl_d) {
		if (fl_mod == 0) {
			if (fl_rm == 6) {
				if (fl_w) {
					src = CPUQueueFetch();
					src |= (CPUQueueFetch()<<8);
				}
				else {
					src = CPUQueueFetch();
				}

				if (edb66 || fl_w)
					*(regptr_dec32[fl_reg]) = src;
				else
					*(regptr_dec[fl_reg]) = (WORD)src;
			}
			else {
				if (fl_w) {
					src = (DWORD)RMAddresswfar(fl_rm,0);
				}
				else {
					src = (DWORD)RMAddressbfar(fl_rm,0);
				}

				if (edb66 || fl_w)
					*(regptr_dec32[fl_reg]) = src;
				else
					*(regptr_dec[fl_reg]) = (WORD)src;
			}
		}
		if (fl_mod == 1) {
			if (fl_w) {
				src = (DWORD)RMAddresswfar(fl_rm,(int)((char)CPUQueueFetch()));
			}
			else {
				src = (DWORD)RMAddressbfar(fl_rm,(int)((char)CPUQueueFetch()));
			}

			if (edb66 || fl_w)
				*(regptr_dec32[fl_reg]) = src;
			else
				*(regptr_dec[fl_reg]) = (WORD)src;
		}
		if (fl_mod == 2) {
			wa = CPUQueueFetch();
			wa |= (CPUQueueFetch()<<8);

			if (fl_w) {
				src = (DWORD)RMAddresswfar(fl_rm,(int)wa);
			}
			else {
				src = (DWORD)RMAddressbfar(fl_rm,(int)wa);
			}

			if (edb66 || fl_w)
				*(regptr_dec32[fl_reg]) = src;
			else
				*(regptr_dec[fl_reg]) = (WORD)src;
		}
	}
	else {
		MessageBox(hwndMain,"BAD","MOVZX",MB_OK);
		code_freerun=0;
	}
}

void Execute386MODREGRM1(void(*callfunc)(void*,void*,int))
{
	WORD w,w2;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
			ireg_eip += 2;
		}
		else {
			w = (WORD)RMAddresswfar(fl_rm,0);
		}
	}
	if (fl_mod == 1) {
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip)));
		w = (WORD)RMAddresswfar(fl_rm,ol);
		ireg_eip++;
	}
	if (fl_mod == 2) {
		ol = (int)((short int)memwordfarptr(ireg_cs,ireg_eip));
		w = (WORD)RMAddresswfar(fl_rm,ol);
		ireg_eip += 2;
	}
	if (fl_mod == 3) {
		w = *regptr_dec[fl_rm];
	}

	w2 = memwordfarptr(ireg_cs,ireg_eip);
	ireg_eip += 2;

	callfunc(&w,&w2,2);

	*regptr_dec[fl_reg] = w2;
}

void Execute386MODREGRM2(void(*callfunc)(void*,void*,int))
{
	WORD w,w2;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
			ireg_eip += 2;
		}
		else {
			w = (WORD)RMAddresswfar(fl_rm,0);
		}
	}
	if (fl_mod == 1) {
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip)));
		w = (WORD)RMAddresswfar(fl_rm,ol);
		ireg_eip++;
	}
	if (fl_mod == 2) {
		ol = (int)((short int)memwordfarptr(ireg_cs,ireg_eip));
		w = (WORD)RMAddresswfar(fl_rm,ol);
		ireg_eip += 2;
	}
	if (fl_mod == 3) {
		w = *regptr_dec[fl_rm];
	}

	w2 = (WORD)membytefarptr(ireg_cs,ireg_eip);
	ireg_eip++;

	callfunc(&w,&w2,2);

	*regptr_dec[fl_reg] = w2;
}

void ExecuteMODREGRMSHXD(void(*callfunc)(void*,void*,int,BYTE))
{
	DWORD dw;
	WORD w;
	BYTE b,shif;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			shif = membytefarptr(ireg_cs,ireg_eip+2);
			if (edb66) {
				dw = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				if (fl_d) {
					callfunc(&dw,regptr_dec32[fl_reg],4,shif);
				}
				else {
					callfunc(regptr_dec32[fl_reg],&dw,4,shif);
					writememdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),dw);
				}
			}
			else if (fl_w) {
				w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				if (fl_d) {
					callfunc(&w,regptr_dec[fl_reg],2,shif);
				}
				else {
					callfunc(regptr_dec[fl_reg],&w,2,shif);
					writememwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),w);
				}
			}
			else {
				b = membytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				if (fl_d) {
					callfunc(&b,regptr_decbyte[fl_reg],1,shif);
				}
				else {
					callfunc(regptr_decbyte[fl_reg],&b,1,shif);
					writemembytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),b);
				}
			}
			ireg_eip += 3;
		}
		else {
			shif = membytefarptr(ireg_cs,ireg_eip);
			if (edb66) {
				dw = RMAddressdfar(fl_rm,0);
				if (fl_d) {
					callfunc(&dw,regptr_dec32[fl_reg],4,shif);
				}
				else {
					callfunc(regptr_dec32[fl_reg],&dw,4,shif);
					RMAddressdfarw(fl_rm,0,dw);
				}
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,0);
				if (fl_d) {
					callfunc(&w,regptr_dec[fl_reg],2,shif);
				}
				else {
					callfunc(regptr_dec[fl_reg],&w,2,shif);
					RMAddresswfarw(fl_rm,0,w);
				}
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,0);
				if (fl_d) {
					callfunc(&b,regptr_decbyte[fl_reg],1,shif);
				}
				else {
					callfunc(regptr_decbyte[fl_reg],&b,1,shif);
					RMAddressbfarw(fl_rm,0,b);
				}
			}
			ireg_eip++;
		}
	}
	if (fl_mod == 1) {
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip)));
		shif = membytefarptr(ireg_cs,ireg_eip+1);
		if (edb66) {
			dw = RMAddressdfar(fl_rm,ol);
			if (fl_d) {
				callfunc(&dw,regptr_dec32[fl_reg],4,shif);
			}
			else {
				callfunc(regptr_dec32[fl_reg],&dw,4,shif);
				RMAddressdfarw(fl_rm,ol,dw);
			}
		}
		else if (fl_w) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			if (fl_d) {
				callfunc(&w,regptr_dec[fl_reg],2,shif);
			}
			else {
				callfunc(regptr_dec[fl_reg],&w,2,shif);
				RMAddresswfarw(fl_rm,ol,w);
			}
		}
		else {
			b = (BYTE)RMAddressbfar(fl_rm,ol);
			if (fl_d) {
				callfunc(&b,regptr_decbyte[fl_reg],1,shif);
			}
			else {
				callfunc(regptr_decbyte[fl_reg],&b,1,shif);
				RMAddressbfarw(fl_rm,ol,b);
			}
		}
		ireg_eip += 2;
	}
	if (fl_mod == 2) {
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip));
		shif = membytefarptr(ireg_cs,ireg_eip+2);
		if (fl_d) {
			if (edb66) {
				dw = RMAddressdfar(fl_rm,ol);
				callfunc(&dw,regptr_dec32[fl_reg],4,shif);
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,ol);
				callfunc(&w,regptr_dec[fl_reg],2,shif);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,ol);
				callfunc(&b,regptr_decbyte[fl_reg],1,shif);
			}
		}
		else {
			if (edb66) {
				dw = RMAddressdfar(fl_rm,ol);
				callfunc(regptr_dec32[fl_reg],&dw,4,shif);
				RMAddressdfarw(fl_rm,ol,dw);
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,ol);
				callfunc(regptr_dec[fl_reg],&w,2,shif);
				RMAddresswfarw(fl_rm,ol,w);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,ol);
				callfunc(regptr_decbyte[fl_reg],&b,1,shif);
				RMAddressbfarw(fl_rm,ol,b);
			}
		}
		ireg_eip += 3;
	}
	if (fl_mod == 3) {
		shif = membytefarptr(ireg_cs,ireg_eip);
		if (edb66) {
			if (fl_d)
				callfunc(regptr_dec32[fl_rm],regptr_dec32[fl_reg],4,shif);
			else
				callfunc(regptr_dec32[fl_reg],regptr_dec32[fl_rm],4,shif);
		}
		else if (fl_w) {
			if (fl_d)
				callfunc(regptr_dec[fl_rm],regptr_dec[fl_reg],2,shif);
			else
				callfunc(regptr_dec[fl_reg],regptr_dec[fl_rm],2,shif);
		}
		else {
			if (fl_d)
				callfunc(regptr_decbyte[fl_rm],regptr_decbyte[fl_reg],1,shif);
			else
				callfunc(regptr_decbyte[fl_reg],regptr_decbyte[fl_rm],1,shif);
		}
		ireg_eip++;
	}
}

void ExecuteMODSREGRM(void(*callfunc)(void*,void*,int))
{
	WORD w;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			if (fl_d) {
				w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				callfunc(&w,regptr_seg[fl_reg],2);
			}
			else {
				callfunc(regptr_seg[fl_reg],&w,2);
				writememwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),w);
			}
			ireg_eip += 2;
		}
		else {
			if (fl_d) {
				w = (WORD)RMAddresswfar(fl_rm,0);
				callfunc(&w,regptr_seg[fl_reg],2);
			}
			else {
				callfunc(regptr_seg[fl_reg],&w,2);
				RMAddresswfarw(fl_rm,0,w);
			}
		}
	}
	if (fl_mod == 1) {
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip)));
		if (fl_d) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(&w,regptr_seg[fl_reg],2);
		}
		else {
			callfunc(regptr_seg[fl_reg],&w,2);
			RMAddresswfarw(fl_rm,ol,w);
		}
		ireg_eip++;
	}
	if (fl_mod == 2) {
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip));
		if (fl_d) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(&w,regptr_seg[fl_reg],2);
		}
		else {
			callfunc(regptr_seg[fl_reg],&w,2);
			RMAddresswfarw(fl_rm,ol,w);
		}
		ireg_eip += 2;
	}
	if (fl_mod == 3) {
		if (fl_d)
			callfunc(regptr_dec[fl_rm],regptr_seg[fl_reg],2);
		else
			callfunc(regptr_seg[fl_reg],regptr_dec[fl_rm],2);
	}
}

void ExecuteMODREGRMXCHG(void(*callfunc)(void*,void*,int))
{
	DWORD dw;
	WORD w;
	BYTE b;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			if (edb66) {
				dw = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				callfunc(&dw,regptr_dec32[fl_reg],4);
				writememdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),dw);
			}
			else if (fl_w) {
				w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				callfunc(&w,regptr_dec[fl_reg],2);
				writememwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),w);
			}
			else {
				b = membytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				callfunc(&b,regptr_decbyte[fl_reg],1);
				writemembytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),b);
			}
			ireg_eip += 2;
		}
		else {
			if (edb66) {
				dw = RMAddressdfar(fl_rm,0);
				callfunc(&dw,regptr_dec32[fl_reg],4);
				RMAddressdfarw(fl_rm,0,dw);
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,0);
				callfunc(&w,regptr_dec[fl_reg],2);
				RMAddresswfarw(fl_rm,0,w);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,0);
				callfunc(&b,regptr_decbyte[fl_reg],1);
				RMAddressbfarw(fl_rm,0,b);
			}
		}
	}
	if (fl_mod == 1) {
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip)));
		if (edb66) {
			dw = RMAddressdfar(fl_rm,ol);
			callfunc(&dw,regptr_dec32[fl_reg],4);
			RMAddressdfarw(fl_rm,ol,dw);
		}
		else if (fl_w) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(&w,regptr_dec[fl_reg],2);
			RMAddresswfarw(fl_rm,ol,w);
		}
		else {
			b = (BYTE)RMAddressbfar(fl_rm,ol);
			callfunc(&b,regptr_decbyte[fl_reg],1);
			RMAddressbfarw(fl_rm,ol,b);
		}
		ireg_eip++;
	}
	if (fl_mod == 2) {
		ol = (int)((short int)memwordfarptr(ireg_cs,ireg_eip));
		if (edb66) {
			dw = RMAddressdfar(fl_rm,ol);
			callfunc(regptr_dec32[fl_reg],&dw,4);
			RMAddressdfarw(fl_rm,ol,dw);
		}
		else if (fl_w) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(regptr_dec[fl_reg],&w,2);
			RMAddresswfarw(fl_rm,ol,w);
		}
		else {
			b = (BYTE)RMAddressbfar(fl_rm,ol);
			callfunc(regptr_decbyte[fl_reg],&b,1);
			RMAddressbfarw(fl_rm,ol,b);
		}
		ireg_eip += 2;
	}
	if (fl_mod == 3) {
		if (edb66)
			callfunc(regptr_dec32[fl_rm],regptr_dec32[fl_reg],4);
		else if (fl_w)
			callfunc(regptr_dec[fl_rm],regptr_dec[fl_reg],2);
		else
			callfunc(regptr_decbyte[fl_rm],regptr_decbyte[fl_reg],1);
	}
}

void ExecuteMODS1RM(void(*callfunc)(void*,void*,int))
{
	DWORD dw;
	WORD w;
	BYTE b;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			ireg_eip += 2;
			if (edb66) {
				dw = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
				callfunc(&dw,NULL,4);
				writememdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2),dw);
			}
			else if (fl_w) {
				w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
				callfunc(&w,NULL,2);
				writememwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2),w);
			}
			else {
				b = membytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
				callfunc(&b,NULL,1);
				writemembytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2),b);
			}
		}
		else {
			if (edb66) {
				dw = RMAddressdfar(fl_rm,0);
				callfunc(&dw,NULL,4);
				RMAddressdfarw(fl_rm,0,dw);
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,0);
				callfunc(&w,NULL,2);
				RMAddresswfarw(fl_rm,0,w);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,0);
				callfunc(&b,NULL,1);
				RMAddressbfarw(fl_rm,0,b);
			}
		}
	}
	if (fl_mod == 1) {
		ireg_eip++;
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip-1)));
		if (edb66) {
			dw = RMAddressdfar(fl_rm,ol);
			callfunc(&dw,NULL,4);
			RMAddressdfarw(fl_rm,ol,dw);
		}
		else if (fl_w) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(&w,NULL,2);
			RMAddresswfarw(fl_rm,ol,w);
		}
		else {
			b = (BYTE)RMAddressbfar(fl_rm,ol);
			callfunc(&b,NULL,1);
			RMAddressbfarw(fl_rm,ol,b);
		}
	}
	if (fl_mod == 2) {
		ireg_eip += 2;
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip-2));
		if (edb66) {
			dw = RMAddressdfar(fl_rm,ol);
			callfunc(&dw,NULL,4);
			RMAddressdfarw(fl_rm,ol,dw);
		}
		else if (fl_w) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(&w,NULL,2);
			RMAddresswfarw(fl_rm,ol,w);
		}
		else {
			b = (BYTE)RMAddressbfar(fl_rm,ol);
			callfunc(&b,NULL,1);
			RMAddressbfarw(fl_rm,ol,b);
		}
	}
	if (fl_mod == 3) {
		if (edb66)
			callfunc(regptr_dec32[fl_rm],NULL,4);
		else if (fl_w)
			callfunc(regptr_dec[fl_rm],NULL,2);
		else
			callfunc(regptr_decbyte[fl_rm],NULL,1);
	}
}

void ExecuteMODS1RMRDONLY(void(*callfunc)(void*,void*,int))
{
	DWORD dw;
	WORD w;
	BYTE b;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			ireg_eip += 2;
			if (edb66) {
				dw = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
				callfunc(&dw,NULL,4);
			}
			else if (fl_w) {
				w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
				callfunc(&w,NULL,2);
			}
			else {
				b = membytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
				callfunc(&b,NULL,1);
			}
		}
		else {
			if (edb66) {
				dw = RMAddressdfar(fl_rm,0);
				callfunc(&dw,NULL,4);
			}
			else if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,0);
				callfunc(&w,NULL,2);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,0);
				callfunc(&b,NULL,1);
			}
		}
	}
	if (fl_mod == 1) {
		ireg_eip++;
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip-1)));
		if (edb66) {
			dw = RMAddressdfar(fl_rm,ol);
			callfunc(&dw,NULL,4);
		}
		else if (fl_w) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(&w,NULL,2);
		}
		else {
			b = (BYTE)RMAddressbfar(fl_rm,ol);
			callfunc(&b,NULL,1);
		}
	}
	if (fl_mod == 2) {
		ireg_eip += 2;
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip-2));
		if (edb66) {
			dw = RMAddressdfar(fl_rm,ol);
			callfunc(&dw,NULL,4);
		}
		else if (fl_w) {
			w = (WORD)RMAddresswfar(fl_rm,ol);
			callfunc(&w,NULL,2);
		}
		else {
			b = (BYTE)RMAddressbfar(fl_rm,ol);
			callfunc(&b,NULL,1);
		}
	}
	if (fl_mod == 3) {
		if (edb66)
			callfunc(regptr_dec32[fl_rm],NULL,4);
		else if (fl_w)
			callfunc(regptr_dec[fl_rm],NULL,2);
		else
			callfunc(regptr_decbyte[fl_rm],NULL,1);
	}
}

void ExecuteMODS1RMD(void(*callfunc)(void*,void*,int))
{
	DWORD dw;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			ireg_eip += 2;
			dw = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
			callfunc(&dw,NULL,2);
			writememdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2),dw);
		}
		else {
			dw = RMAddressdfar(fl_rm,0);
			callfunc(&dw,NULL,2);
			RMAddressdfarw(fl_rm,0,dw);
		}
	}
	if (fl_mod == 1) {
		ireg_eip++;
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip-1)));
		dw = RMAddressdfar(fl_rm,ol);
		callfunc(&dw,NULL,2);
		RMAddressdfarw(fl_rm,ol,dw);
	}
	if (fl_mod == 2) {
		ireg_eip += 2;
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip-2));
		dw = RMAddressdfar(fl_rm,ol);
		callfunc(&dw,NULL,2);
		RMAddressdfarw(fl_rm,ol,dw);
	}
	if (fl_mod == 3) {
// illegal mode
	}
}

void ExecuteMODS1RMDRDONLY(void(*callfunc)(void*,void*,int))
{
	DWORD dw;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			ireg_eip += 2;
			dw = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
			callfunc(&dw,NULL,2);
		}
		else {
			dw = RMAddressdfar(fl_rm,0);
			callfunc(&dw,NULL,2);
		}
	}
	if (fl_mod == 1) {
		ireg_eip++;
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip-1)));
		dw = RMAddressdfar(fl_rm,ol);
		callfunc(&dw,NULL,2);
	}
	if (fl_mod == 2) {
		ireg_eip += 2;
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip-2));
		dw = RMAddressdfar(fl_rm,ol);
		callfunc(&dw,NULL,2);
	}
	if (fl_mod == 3) {
// illegal mode
	}
}

void ExecuteMODS1RMF(void(*callfunc)(void*,void*,int))
{
	DWORD dw,dw2;
	WORD w;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			ireg_eip += 2;
			w = memwordfarptr(ireg_cs,ireg_eip-2);
			dw = memdwordfarptr(*ireg_dataseg,w);
			dw2 = memdwordfarptr(*ireg_dataseg,w+4);
			callfunc(&dw,&dw2,6);
			writememdwordfarptr(*ireg_dataseg,w,dw);
			writememdwordfarptr(*ireg_dataseg,w+4,dw2);
		}
		else {
			dw = RMAddressdfar(fl_rm,0);
			dw2 |= RMAddressdfar(fl_rm,4);
			callfunc(&dw,&dw2,6);
			RMAddressdfarw(fl_rm,0,dw);
			RMAddressdfarw(fl_rm,4,dw2);
		}
	}
/*	if (fl_mod == 1) {
		ireg_eip++;
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip-1)));
		qw = RMAddressdfar(fl_rm,ol);
		qw |= (RMAddressdfar(fl_rm,ol+4)<<32);
		callfunc(&qw,NULL,6);
		RMAddressdfarw(fl_rm,ol,qw);
		RMAddressdfarw(fl_rm,ol+4,qw>>32);
	}
	if (fl_mod == 2) {
		ireg_eip += 2;
		ol = (int)((short int)memwordfarptr(ireg_cs,ireg_eip-2));
		qw = RMAddressdfar(fl_rm,ol);
		qw |= (RMAddressdfar(fl_rm,ol)<<32);
		callfunc(&qw,NULL,6);
		RMAddressdfarw(fl_rm,ol,qw);
		RMAddressdfarw(fl_rm,ol+4,qw>>32);
	}
	if (fl_mod == 3) {
// illegal mode
	} */
}

void ExecuteMODLDS(void(*callfunc)(void*,void*,int))
{
	DWORD dw;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			ireg_eip += 2;
			dw = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip-2));
			callfunc(&dw,regptr_dec[fl_reg],2);
		}
		else {
			dw = RMAddressdfar(fl_rm,0);
			callfunc(&dw,regptr_dec[fl_reg],2);
		}
	}
	if (fl_mod == 1) {
		ireg_eip++;
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip-1)));
		dw = RMAddressdfar(fl_rm,ol);
		callfunc(&dw,regptr_dec[fl_reg],2);
	}
	if (fl_mod == 2) {
		ireg_eip += 2;
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip-2));
		dw = RMAddressdfar(fl_rm,ol);
		callfunc(&dw,regptr_dec[fl_reg],2);
	}
	if (fl_mod == 3) {
// illegal mode
	}
}

void ExecuteMODLEA(void(*callfunc)(void*,void*,int))
{
	WORD w;
	int ol;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			ireg_eip += 2;
			w = memwordfarptr(ireg_cs,ireg_eip-2);
			callfunc(&w,regptr_dec[fl_reg],2);
		}
		else {
			w = (WORD)RMEAddress(fl_rm,0);
			callfunc(&w,regptr_dec[fl_reg],2);
		}
	}
	if (fl_mod == 1) {
		ireg_eip++;
		ol = ((int)((char)membytefarptr(ireg_cs,ireg_eip-1)));
		w = (WORD)RMEAddress(fl_rm,ol);
		callfunc(&w,regptr_dec[fl_reg],2);
	}
	if (fl_mod == 2) {
		ireg_eip += 2;
		ol = (int)(memwordfarptr(ireg_cs,ireg_eip-2));
		w = (WORD)RMEAddress(fl_rm,ol);
		callfunc(&w,regptr_dec[fl_reg],2);
	}
	if (fl_mod == 3) {
// illegal mode
	}
}

void ExecuteMODSHII(void(*callfunc)(void*,void*,int))
{
	DWORD d;
	WORD w;
	BYTE b;
	int o;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			if (edb67)	exec_imm1 = membytefarptr(ireg_cs,ireg_eip+4);
			else		exec_imm1 = membytefarptr(ireg_cs,ireg_eip+2);
			if (fl_w) {
				if (edb66) {
					if (edb67) 
						d = memdwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
					else
						d = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
					callfunc(&d,NULL,4);
					if (edb67) 
						writememdwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),d);
					else
						writememdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),d);
				}
				else {
					if (edb67) 
						w = memwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
					else
						w = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
					callfunc(&w,NULL,2);
					if (edb67) 
						writememwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),w);
					else
						writememwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),w);
				}
			}
			else {
				if (edb67) 
					b = membytefarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
				else
					b = membytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
				callfunc(&b,NULL,1);
				if (edb67) 
					writemembytefarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),b);
				else
					writemembytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),b);
			}
			ireg_eip += (edb67 ? 5 : 3);
		}
		else {
			exec_imm1 = membytefarptr(ireg_cs,ireg_eip);
			if (fl_w) {
				if (edb66) {
					if (!edb67) {
						d = RMAddressdfar(fl_rm,0);
					}
					else {
						d = RMAddressdfare(fl_rm,0);
					}
					callfunc(&d,NULL,4);
					if (!edb67) {
						RMAddressdfarw(fl_rm,0,d);
					}
					else {
						RMAddressdfarew(fl_rm,0,d);
					}
				}
				else {
					if (!edb67) {
						w = (WORD)RMAddresswfar(fl_rm,0);
					}
					else {
						w = (WORD)RMAddresswfare(fl_rm,0);
					}
					callfunc(&w,NULL,2);
					if (!edb67) {
						RMAddresswfarw(fl_rm,0,w);
					}
					else {
						RMAddresswfarew(fl_rm,0,w);
					}
				}
			}
			else {
				if (edb67) {
					b = (BYTE)RMAddressbfare(fl_rm,0);
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,0);
				}
				callfunc(&b,NULL,1);
				if (edb67) {
					RMAddressbfarew(fl_rm,0,b);
				}
				else {
					RMAddressbfarw(fl_rm,0,b);
				}
			}
			ireg_eip++;
		}
	}
	if (fl_mod == 1) {
		exec_imm1 = membytefarptr(ireg_cs,ireg_eip+1);
		o = (int)((char)membytefarptr(ireg_cs,ireg_eip));
		if (fl_w) {
			if (edb66) {
				if (!edb67) {
					d = RMAddressdfar(fl_rm,o);
				}
				else {
					d = RMAddressdfare(fl_rm,o);
				}
				callfunc(&d,NULL,4);
			}
			else {
				if (!edb67) {
					w = (WORD)RMAddresswfar(fl_rm,o);
				}
				else {
					w = (WORD)RMAddresswfare(fl_rm,o);
				}
				callfunc(&w,NULL,2);
			}
		}
		else {
			if (edb67) {
				b = (BYTE)RMAddressbfare(fl_rm,o);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,o);
			}
			callfunc(&b,NULL,1);
		}
		ireg_eip += 2;
	}
	if (fl_mod == 2) {
		exec_imm1 = membytefarptr(ireg_cs,ireg_eip+2);
		o = (int)(memwordfarptr(ireg_cs,ireg_eip));
		if (fl_w) {
			if (edb66) {
				if (!edb67) {
					d = RMAddressdfar(fl_rm,o);
				}
				else {
					d = RMAddressdfare(fl_rm,o);
				}
				callfunc(&d,NULL,4);
			}
			else {
				if (!edb67) {
					w = (WORD)RMAddresswfar(fl_rm,o);
				}
				else {
					w = (WORD)RMAddresswfare(fl_rm,o);
				}
				callfunc(&w,NULL,2);
			}
		}
		else {
			if (edb67) {
				b = (BYTE)RMAddressbfare(fl_rm,o);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,o);
			}
			callfunc(&b,NULL,1);
		}
		ireg_eip += 3;
	}
	if (fl_mod == 3) {
		exec_imm1 = membytefarptr(ireg_cs,ireg_eip);
		if (fl_w) {
			if (edb66)	callfunc(regptr_dec32[fl_rm],NULL,4);
			else		callfunc(regptr_dec[fl_rm],NULL,2);
		}
		else {
			callfunc(regptr_decbyte[fl_rm],NULL,1);
		}
		ireg_eip++;
	}

	edb66=0;
	edb67=0;
}

void ExecuteMOD010RM(void(*callfunc)(void*,void*,int))
{
	DWORD dw,dw2;
	WORD w,w2;
	BYTE b,b2;
	int ad;

	if (edb67 && fl_mod != 3 && fl_rm == 4) {
		MessageBox(hwndMain,"ExecuteMOD010RM() - SIB bytes unsupported","Program Error",MB_OK);
		SignalInterrupt16(0x06);		// Invalid opcode! (I DON'T DO SIB BYTES!)
	}

	if (edb67) {
		edb67=0;
		if (fl_mod == 0) {
			if (fl_rm == 5) {
				if (edb66) {
					edb66=0;
					if (fl_s) {
						dw = (DWORD)((short int)((char)membytefarptr(ireg_cs,ireg_eip + 4)));
						dw2 = memdwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
						callfunc(&dw,&dw2,4);
						writememdwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),dw2);
						ireg_eip += 5;
					}
					else {
						dw = memdwordfarptr(ireg_cs,ireg_eip + 4);
						dw2 = memdwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
						callfunc(&dw,&dw2,4);
						writememdwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),dw2);
						ireg_eip += 8;
					}
				}
				else if (fl_w) {
					if (fl_s) {
						w = (WORD)((short int)((char)membytefarptr(ireg_cs,ireg_eip + 4)));
						w2 = memwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
						callfunc(&w,&w2,2);
						writememwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),w2);
						ireg_eip += 5;
					}
					else {
						w = memwordfarptr(ireg_cs,ireg_eip + 4);
						w2 = memwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
						callfunc(&w,&w2,2);
						writememwordfarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),w2);
						ireg_eip += 6;
					}
				}
				else {
					b = membytefarptr(ireg_cs,ireg_eip + 4);
					b2 = membytefarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip));
					callfunc(&b,&b2,1);
					writemembytefarptr(*ireg_dataseg,memdwordfarptr(ireg_cs,ireg_eip),b2);
					ireg_eip += 5;
				}
			}
			else {
				if (edb66) {
					edb66=0;
					dw = RMAddressdfar32(fl_rm,0);
					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						dw2 = (DWORD)((char)b2);
						ireg_eip++;
					}
					else {
						dw2 = memdwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 4;
					}
					callfunc(&dw2,&dw,4);

					RMAddressdfarw32(fl_rm,0,dw);
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar32(fl_rm,0);
					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						w2 = (WORD)((char)b2);
						ireg_eip++;
					}
					else {
						w2 = memwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 2;
					}
					callfunc(&w2,&w,2);

					RMAddresswfarw32(fl_rm,0,w);
				}
				else {
					b = (BYTE)RMAddressbfar32(fl_rm,0);
					b2 = membytefarptr(ireg_cs,ireg_eip);
					ireg_eip++;
					callfunc(&b2,&b,1);

					RMAddressbfarw32(fl_rm,0,b);
				}
			}
		}	
		if (fl_mod == 1) {
			ad = (int)((char)membytefarptr(ireg_cs,ireg_eip));
			ireg_eip += 1;
			if (fl_w) {
				w = (WORD)RMAddresswfar(fl_rm,ad);
				if (fl_s) {
					b2 = membytefarptr(ireg_cs,ireg_eip);
					w2 = (WORD)((char)b2);
					ireg_eip++;
				}
				else {
					w2 = memwordfarptr(ireg_cs,ireg_eip);
					ireg_eip += 2;
				}
				callfunc(&w2,&w,2);

				RMAddresswfarw(fl_rm,ad,w);
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,ad);
				b2 = membytefarptr(ireg_cs,ireg_eip);
				ireg_eip++;
				callfunc(&b2,&b,1);

				RMAddressbfarw(fl_rm,ad,b);
			}
		}
		if (fl_mod == 2) {
			ad = (int)(memwordfarptr(ireg_cs,ireg_eip));
			ireg_eip += 2;
			if (fl_w) {
				w = (WORD)RMAddresswfar32(fl_rm,ad);
				if (fl_s) {
					b2 = membytefarptr(ireg_cs,ireg_eip);
					w2 = (WORD)((char)b2);
					ireg_eip++;
				}
				else {
					w2 = memwordfarptr(ireg_cs,ireg_eip);
					ireg_eip += 2;
				}
				callfunc(&w2,&w,2);

				RMAddresswfarw32(fl_rm,ad,w);
			}
			else {
				b = (BYTE)RMAddressbfar32(fl_rm,ad);
				b2 = membytefarptr(ireg_cs,ireg_eip);
				ireg_eip++;
				callfunc(&b2,&b,1);

				RMAddressbfarw32(fl_rm,ad,b);
			}
		}
		if (fl_mod == 3) {
			if (fl_w) {
				if (fl_s) {
					b = membytefarptr(ireg_cs,ireg_eip++);
					__asm {
						mov		al,b
						movsx	bx,al
						mov		w,bx
					}
				}
				else {
					w = memwordfarptr(ireg_cs,ireg_eip);
					ireg_eip += 2;
				}
				callfunc(&w,regptr_dec[fl_rm],2);
			}
			else {
				b = membytefarptr(ireg_cs,ireg_eip++);
				callfunc(&b,regptr_decbyte[fl_rm],1);
			}
		}
	}
	else {
		if (fl_mod == 0) {
			if (fl_rm == 6) {
				if (edb66) {
					edb66=0;
					if (fl_s) {
						dw = (DWORD)((int)((char)membytefarptr(ireg_cs,ireg_eip + 2)));
						dw2 = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
						callfunc(&dw,&dw2,4);
						writememdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),dw2);
						ireg_eip += 3;
					}
					else {
						dw = memdwordfarptr(ireg_cs,ireg_eip + 2);
						dw2 = memdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
						callfunc(&dw,&dw2,4);
						writememdwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),dw2);
						ireg_eip += 6;
					}
				}
				else if (fl_w) {
					if (fl_s) {
						w = (WORD)((short int)((char)membytefarptr(ireg_cs,ireg_eip + 2)));
						w2 = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
						callfunc(&w,&w2,2);
						writememwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),w2);
						ireg_eip += 3;
					}
					else {
						w = memwordfarptr(ireg_cs,ireg_eip + 2);
						w2 = memwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
						callfunc(&w,&w2,2);
						writememwordfarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),w2);
						ireg_eip += 4;
					}
				}
				else {
					b = membytefarptr(ireg_cs,ireg_eip + 2);
					b2 = membytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip));
					callfunc(&b,&b2,1);
					writemembytefarptr(*ireg_dataseg,memwordfarptr(ireg_cs,ireg_eip),b2);
					ireg_eip += 3;
				}
			}
			else {
				if (edb66) {
					edb66=0;
					dw = RMAddressdfar(fl_rm,0);

					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						dw2 = (DWORD)((char)b2);
						ireg_eip++;
					}
					else {
						dw2 = memdwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 4;
					}
					callfunc(&dw2,&dw,4);

					RMAddressdfarw(fl_rm,0,dw);
				}
				else if (fl_w) {
					w = (WORD)RMAddresswfar(fl_rm,0);

					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						w2 = (WORD)((char)b2);
						ireg_eip++;
					}
					else {
						w2 = memwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 2;
					}
					callfunc(&w2,&w,2);

					RMAddresswfarw(fl_rm,0,w);
				}
				else {
					b = (BYTE)RMAddressbfar(fl_rm,0);
					b2 = membytefarptr(ireg_cs,ireg_eip);
					ireg_eip++;
					callfunc(&b2,&b,1);

					RMAddressbfarw(fl_rm,0,b);
				}
			}
		}	
		if (fl_mod == 1) {
			ad = (int)((char)membytefarptr(ireg_cs,ireg_eip));
			ireg_eip += 1;
			if (fl_w) {
				if (edb66) {
					dw = RMAddressdfar(fl_rm,ad);
					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						dw2 = (DWORD)((char)b2);
						ireg_eip++;
					}
					else {
						dw2 = memdwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 4;
					}
					callfunc(&dw2,&dw,4);

					RMAddressdfarw(fl_rm,ad,dw);
				}
				else {
					w = (WORD)RMAddresswfar(fl_rm,ad);
					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						w2 = (WORD)((char)b2);
						ireg_eip++;
					}
					else {
						w2 = memwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 2;
					}
					callfunc(&w2,&w,2);

					RMAddresswfarw(fl_rm,ad,w);
				}
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,ad);
				b2 = membytefarptr(ireg_cs,ireg_eip);
				ireg_eip++;
				callfunc(&b2,&b,1);

				RMAddressbfarw(fl_rm,ad,b);
			}
		}
		if (fl_mod == 2) {
			ad = (int)(memwordfarptr(ireg_cs,ireg_eip));
			ireg_eip += 2;
			if (fl_w) {
				if (edb66) {
					dw = RMAddressdfar(fl_rm,ad);
					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						dw2 = (DWORD)((char)b2);
						ireg_eip++;
					}
					else {
						dw2 = memdwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 4;
					}
					callfunc(&dw2,&dw,4);

					RMAddressdfarw(fl_rm,ad,dw);
				}
				else {
					w = (WORD)RMAddresswfar(fl_rm,ad);
					if (fl_s) {
						b2 = membytefarptr(ireg_cs,ireg_eip);
						w2 = (WORD)((char)b2);
						ireg_eip++;
					}
					else {
						w2 = memwordfarptr(ireg_cs,ireg_eip);
						ireg_eip += 2;
					}
					callfunc(&w2,&w,2);

					RMAddresswfarw(fl_rm,ad,w);
				}
			}
			else {
				b = (BYTE)RMAddressbfar(fl_rm,ad);
				b2 = membytefarptr(ireg_cs,ireg_eip);
				ireg_eip++;
				callfunc(&b2,&b,1);

				RMAddressbfarw(fl_rm,ad,b);
			}
		}
		if (fl_mod == 3) {
			if (edb66) {
				edb66=0;
				if (fl_s) {
					b = membytefarptr(ireg_cs,ireg_eip++);
					__asm {
						mov		al,b
						movsx	ebx,al
						mov		dw,ebx
					}
				}
				else {
					dw = memdwordfarptr(ireg_cs,ireg_eip);
					ireg_eip += 4;
				}
				callfunc(&dw,regptr_dec32[fl_rm],4);
			}
			else if (fl_w) {
				if (fl_s) {
					b = membytefarptr(ireg_cs,ireg_eip++);
					__asm {
						mov		al,b
						movsx	bx,al
						mov		w,bx
					}
				}
				else {
					w = memwordfarptr(ireg_cs,ireg_eip);
					ireg_eip += 2;
				}
				callfunc(&w,regptr_dec[fl_rm],2);
			}
			else {
				b = membytefarptr(ireg_cs,ireg_eip++);
				callfunc(&b,regptr_decbyte[fl_rm],1);
			}
		}
	}
}
