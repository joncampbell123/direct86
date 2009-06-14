
#include "global.h"
#include "cpuexec.h"
#include "cpumodel.h"
#include "cpuqueue.h"

MODELOPCODE*		mdl_cpu_now=NULL;
DWORD				exec_begin_cs,exec_begin_eip;
char				segment_prefix_dec[32];

// CPU model table handling

void set_cpu_model(MODELOPCODE *model)
{
	mdl_cpu_now=model;
}

int model_execute_opcode()
{
	BYTE op;
	MODELOPCODE *table;
    int oc,ag;

	exec_begin_cs=ireg_cs;
	exec_begin_eip=ireg_eip;

	table = mdl_cpu_now;
	oc = 0;
	ag=1;
	while (oc < 16 && table && ag) {
		op = CPUQueueFetch();
		oc++;

        ag = table[(int)op].execute(op);
		if (!ag) {
            table = NULL;
		}
        else if (table[(int)op].lastbyte) {
			table = NULL;
		}
		else {
            if (table[(int)op].ref_table) table = (MODELOPCODE*)table[(int)op].ref_table;
		}
	}

	return ag;
}

int model_decode_opcode(char *buffer)
{
	BYTE op;
	MODELOPCODE *table;
	int oc,ag,xx;
	char *tmptr,*otmpr;

	segment_prefix_dec[0] = 0;
	otmpr = tmptr = buffer;
	table = mdl_cpu_now;
	oc = 0;
	ag=1;
	while (oc < 16 && table && ag) {
		op = CPUDecoderQueueFetch();
		oc++;

		ag = table[(int)op].decode(op,tmptr);
		if (!ag) {
			table = NULL;
		}
		else if (table[(int)op].lastbyte) {
			table = NULL;
		}
		else {
            if (table[(int)op].ref_table) table = (MODELOPCODE*)table[(int)op].ref_table;
			tmptr += strlen(tmptr);

			xx = (int)(tmptr - otmpr);
			while (xx & 7) {
				xx++;
				*tmptr++ = ' ';
			}
			*tmptr = 0;
		}
	}

	return ag;
}

int mdl_exec_default(BYTE b)
{
	return 0;
}

int mdl_dec_default(BYTE b,char *buf)
{
	return 0;
}
