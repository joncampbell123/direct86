
#include "global.h"
#include "addrbus.h"
#include "mother.h"
#include "cpuqueue.h"
#include "cpumodel.h"
#include "cpudec.h"
#include "cpuexec.h"
#include "direct86.h"
#include "interrm.h"
#include "ram.h"
#include "naming.h"
#include <stdio.h>

DWORD						debug_window_offset32=0;
DWORD						debug_window_offset_cs=0;
DWORD						debug_window_offset_ip=0;
BOOL						fl_d,fl_w,fl_s;
int							fl_mod,fl_reg,fl_rm,fl_fop;
DWORD						fl_bytes;
BOOL						db66=0,db67=0,db66a=0,db67a=0;
char						intcbuf[256];
char						fl_prefix[256];
char *regs8[] =				{"AL","CL","DL","BL","AH","CH","DH","BH"};
char *regs16[] =			{"AX","CX","DX","BX","SP","BP","SI","DI"};
char *regs32[] =			{"EAX","ECX","EDX","EBX","ESP","EBP","ESI","EDI"};
char *regsegs[] =			{"ES","CS","SS","DS","FS","GS","?6","?7"};
char *bptr =				"BYTE PTR ";
char *wptr =				"WORD PTR ";
char *fptr =				"FWORD PTR ";
char *dptr =				"DWORD PTR ";
char *displ[] =				{"BX+SI","BX+DI","BP+SI","BP+DI","SI","DI","BP","BX"};
char *displ32[] =			{"EBX+ESI","EBX+EDI","EBP+ESI","EBP+EDI","ESI","EDI","EBP","EBX"};
char *displ_67[] =			{"EAX","ECX","EDX","EBX","EBX+EAX","EBP","ESI","EDI"};
char *conjmp[] =			{"JO","JNO",
							"JB","JAE",
							"JZ","JNZ",
							"JBE","JA",
							"JS","JNS",
							"JP","JNP",
							"JL","JGE",
							"JLE","JG"};
char *conjmpnear[]=			{"JO","JNO",
							"JB","JNB",
							"JZ","JNZ",
							"JBE","JNBE",
							"JS","JNS",
							"JP","JNP",
							"JL","JNL",
							"JLE","JNLE"};

void DecompileBytes(DWORD *o_cs,DWORD *o_ip,char *s)
{
	BOOL pr=0;
	int ag;

	if (!CPUpower) {
		strcpy(s,"CPU not powered");
		(*o_ip)++;
		return;
	}

// Make ovious our BIOS "patches"
	pr=FALSE;
	if (*o_cs == 0xF000) {
// are we at an address call for INT 00h (F000:EC00h)
		if (*o_ip == 0xEC00) {
			sprintf(s,"Built-in BIOS handler - division by 0");
			pr=TRUE;
		}
// are we at an address call for IRQ 1 (F000:CC11h)
		if (*o_ip == 0xCC11) {
			sprintf(s,"Built-in BIOS handler - IRQ #1");
			pr=TRUE;
		}
// are we at an address call for INT 10h (F000:AB10h)
		if (*o_ip == 0xAB10) {
			sprintf(s,"Built-in BIOS service - INT 10h Video BIOS");
			pr=TRUE;
		}
// are we at an address call for INT 11h (F000:AB11h)
		if (*o_ip == 0xAB11) {
			sprintf(s,"Built-in BIOS service - INT 11h Equipment list");
			pr=TRUE;
		}
// are we at an address call for INT 13h (F000:AB13h)
		if (*o_ip == 0xAB13) {
			sprintf(s,"Built-in BIOS service - INT 13h Diskette BIOS");
			pr=TRUE;
		}
// are we at an address call for INT 14h (F000:AB14h)
		if (*o_ip == 0xAB14) {
			sprintf(s,"Built-in BIOS service - INT 14h Serial BIOS");
			pr=TRUE;
		}
// are we at an address call for INT 15h (F000:AB15h)
		if (*o_ip == 0xAB15) {
			sprintf(s,"Built-in BIOS service - INT 15h Misceallaneus BIOS");
			pr=TRUE;
		}
// are we at an address call for INT 16h (F000:AB16h)
		if (*o_ip == 0xAB16) {
			sprintf(s,"Built-in BIOS service - INT 16h Keyboard BIOS");
			pr=TRUE;
		}
// are we at an address call for INT 17h (F000:AB17h)
		if (*o_ip == 0xAB17) {
			sprintf(s,"Built-in BIOS service - INT 17h Printer BIOS");
			pr=TRUE;
		}
// are we at an address call for INT 1Ah (F000:AB1Ah)
		if (*o_ip == 0xAB1A) {
			sprintf(s,"Built-in BIOS service - INT 1Ah Date/Time BIOS");
			pr=TRUE;
		}
	}

	if (pr) {
		(*o_ip)++;
		return;
	}

	BeginDecoderQueue(*o_cs,*o_ip);
	s[0]=0; ag=model_decode_opcode(s);
	EndDecoderQueue(o_cs,o_ip);
	if (ag) return;
	strcat(s," (Unknown)");

	return;
}

extern DWORD				name_assoc_mem_should_be_ofs;
extern DWORD				name_assoc_mem_should_be_seg;

void DecompileToFile(DWORD cs,DWORD from_ip,DWORD to_ip,char *path)
{
	FILE *fp;
	char buf[2048];
	char commentbuffer[1024];
	DWORD ipn,oipn;
	int gna;

	fp=fopen(path,"wb");
	if (!fp) return;

	oipn=from_ip;
	for (ipn=from_ip;ipn <= to_ip;) {
		gna=GetSkippedItems(cs,oipn,cs,ipn,NULL,&ipn);
		oipn=ipn;
		gna=GetNamingMemAssocStrPreIns(cs,ipn,buf,500);
		if (gna) fprintf(fp,"\n%s\n",buf);

		if (gna < 2) {
			DecompileBytes(&cs,&ipn,buf);

			if (GetNamingMemAssocStrComments(cs,oipn,commentbuffer,1000)) {
				fprintf(fp,"%04X:%04X: %-39s; %s\n",cs,oipn,buf,commentbuffer);
			}
			else {
				fprintf(fp,"%04X:%04X: %s\n",cs,oipn,buf);
			}

			gna=GetNamingMemAssocStrPostIns(cs,oipn,buf,500);
			if (gna) fprintf(fp,"%s\n\n",buf);
		}
		else {
			ipn = name_assoc_mem_should_be_ofs + ((name_assoc_mem_should_be_seg - cs)<<4);
		}
	}

	fclose(fp);
}
