
#include "global.h"
#include <stdio.h>
#include <string.h>
#include "interrm.h"
#include "cpudec.h"
#include "cpuexec.h"
#include "cpuqueue.h"
#include "cpumodel.h"
#include "addrbus.h"

void InterpretMODREGRM(char *insname)
{
	char argv1[32],argv2[32];
	char c;
    WORD w;
    BYTE b;

	argv1[0] = argv2[0] = 0;
    if (fl_mod == 0) {
        if (fl_rm == 6) {
            w = (WORD)CPUDecoderQueueFetch();
            w |= ((WORD)CPUDecoderQueueFetch())<<8;

            if (db66) {
                sprintf(argv1,"DWORD PTR %s[%04Xh]",segment_prefix_dec,w);
                sprintf(argv2,"%s",regs32[fl_reg]);
            }
            else if (fl_w) {
                sprintf(argv1,"WORD PTR %s[%04Xh]",segment_prefix_dec,w);
                sprintf(argv2,"%s",regs16[fl_reg]);
            }
            else {
                sprintf(argv1,"BYTE PTR %s[%04Xh]",segment_prefix_dec,w);
                sprintf(argv2,"%s",regs8[fl_reg]);
            }
        }
        else {
            if (db66) {
                sprintf(argv1,"DWORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
                sprintf(argv2,"%s",regs32[fl_reg]);
            }
            else if (fl_w) {
                sprintf(argv1,"WORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
                sprintf(argv2,"%s",regs16[fl_reg]);
            }
            else {
                sprintf(argv1,"BYTE PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
                sprintf(argv2,"%s",regs8[fl_reg]);
            }
        }
    }
    if (fl_mod == 1) {
        b = CPUDecoderQueueFetch();
        if (b & 0x80)       c = '-';
        else                c = '+';

        if (db66) {
            sprintf(argv1,"DWORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((signed char)b));
            sprintf(argv2,"%s",regs32[fl_reg]);
        }
        else if (fl_w) {
            sprintf(argv1,"WORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((signed char)b));
            sprintf(argv2,"%s",regs16[fl_reg]);
        }
        else {
            sprintf(argv1,"BYTE PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((signed char)b));
            sprintf(argv2,"%s",regs8[fl_reg]);
        }
    }
    if (fl_mod == 2) {
        w = (WORD)CPUDecoderQueueFetch();
        w |= ((WORD)CPUDecoderQueueFetch())<<8;

        if (db66) {
            sprintf(argv1,"WORD PTR %s[%s+%04Xh]",segment_prefix_dec,displ[fl_rm],w);
            sprintf(argv2,"%s",regs16[fl_reg]);
        }
        else if (fl_w) {
            sprintf(argv1,"WORD PTR %s[%s+%04Xh]",segment_prefix_dec,displ[fl_rm],w);
            sprintf(argv2,"%s",regs16[fl_reg]);
        }
        else {
            sprintf(argv1,"BYTE PTR %s[%s+%04Xh]",segment_prefix_dec,displ[fl_rm],w);
            sprintf(argv2,"%s",regs8[fl_reg]);
        }
    }
    if (fl_mod == 3) {
        if (db66) {
            db66a=1;
            sprintf(argv1,"%s",regs32[fl_rm]);
            sprintf(argv2,"%s",regs32[fl_reg]);
        }
        else if (fl_w) {
            sprintf(argv1,"%s",regs16[fl_rm]);
            sprintf(argv2,"%s",regs16[fl_reg]);
        }
        else {
            sprintf(argv1,"%s",regs8[fl_rm]);
            sprintf(argv2,"%s",regs8[fl_reg]);
        }
    }

	if (fl_d)	sprintf(intcbuf,"%-7s %s,%s",insname,argv2,argv1);
	else		sprintf(intcbuf,"%-7s %s,%s",insname,argv1,argv2);

    return;
}

// special interpreter code for the MOVZX/MOVSX instructions
void InterpretMODMOVZ(char *insname)
{
	char argv1[32],argv2[32];
	char c;
    DWORD o=0,o2=0;

	argv1[0] = argv2[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			if (fl_w) {
				sprintf(argv1,"WORD PTR [%04Xh]",memwordlinear(o)); o += 2;
			}
			else {
				sprintf(argv1,"BYTE PTR [%04Xh]",memwordlinear(o)); o += 2;
			}

			if (db66 || fl_w)
				sprintf(argv2,"%s",regs32[fl_reg]);
			else
				sprintf(argv2,"%s",regs16[fl_reg]);
		}
		else {
			if (fl_w)
				sprintf(argv1,"WORD PTR [%s]",displ[fl_rm]);
			else
				sprintf(argv1,"BYTE PTR [%s]",displ[fl_rm]);

			if (db66 || fl_w)
				sprintf(argv2,"%s",regs32[fl_reg]);
			else
				sprintf(argv2,"%s",regs16[fl_reg]);
		}
	}
	if (fl_mod == 1) {
		if (membytelinear(o) & 0x80)
			c = '-';
		else
			c = '+';

		if (fl_w)
			sprintf(argv1,"WORD PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)membytelinear(o)));
		else
			sprintf(argv1,"BYTE PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)membytelinear(o)));

		if (db66 || fl_w)
			sprintf(argv2,"%s",regs32[fl_reg]);
		else
			sprintf(argv2,"%s",regs16[fl_reg]);

		o++;
	}
	if (fl_mod == 2) {
		if (fl_w)
			sprintf(argv1,"WORD PTR [%s+%04Xh]",displ[fl_rm],memwordlinear(o));
		else
			sprintf(argv1,"BYTE PTR [%s+%04Xh]",displ[fl_rm],memwordlinear(o));

		if (db66 || fl_w)
			sprintf(argv2,"%s",regs32[fl_reg]);
		else
			sprintf(argv2,"%s",regs16[fl_reg]);

		o += 2;
	}
	if (fl_mod == 3) {
		fl_d=0;

		if (db66 || fl_w)
			sprintf(argv1,"%s",regs32[fl_reg]);
		else
			sprintf(argv1,"%s",regs16[fl_reg]);

		if (fl_w)
			sprintf(argv2,"%s",regs16[fl_rm]);
		else
			sprintf(argv2,"%s",regs8[fl_rm]);
	}
	db66a=1;

	if (fl_d)
		sprintf(intcbuf,"%s %s%s,%s",insname,fl_prefix,argv2,argv1);
	else
		sprintf(intcbuf,"%s %s%s,%s",insname,fl_prefix,argv1,argv2);

	fl_bytes = o-o2;
    return;
}

void Interpret386MODREGRM1(char *insname)
{
	char argv1[32],argv2[32];
	char c;
	WORD w1;
	BYTE b1;

	argv1[0] = argv2[0] = 0;

	sprintf(argv1,"%s %s,",insname,regs16[fl_reg]);

	if (fl_mod == 0) {
		if (fl_rm == 6)	{
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
			sprintf(argv2,"WORD PTR [%04Xh]",w1);
		}
		else {
			sprintf(argv2,"WORD PTR [%s]",displ[fl_rm]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';
		sprintf(argv2,"WORD PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)b1));
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(argv2,"WORD PTR [%s+%04Xh]",displ[fl_rm],w1);
	}
	if (fl_mod == 3) {
		sprintf(argv2,"%s",regs16[fl_rm]);
	}

	w1 = (WORD)CPUDecoderQueueFetch();
	w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
	strcpy(intcbuf,argv1);
	strcat(intcbuf,argv2);
	sprintf(argv1,",%04X",w1);
	strcat(intcbuf,argv1);

    return;
}

void Interpret386MODREGRM2(char *insname)
{
	char argv1[32],argv2[32];
	char c;
	WORD w1;
	BYTE b1;

	argv1[0] = argv2[0] = 0;

	sprintf(argv1,"%s %s,",insname,regs16[fl_reg]);

	if (fl_mod == 0) {
		if (fl_rm == 6)	{
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
			sprintf(argv2,"WORD PTR [%04Xh]",w1);
		}
		else {
			sprintf(argv2,"WORD PTR [%s]",displ[fl_rm]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';
		sprintf(argv2,"WORD PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)b1));
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		sprintf(argv2,"WORD PTR [%s+%04Xh]",displ[fl_rm],w1);
	}
	if (fl_mod == 3) {
		sprintf(argv2,"%s",regs16[fl_rm]);
	}

	b1 = CPUDecoderQueueFetch();
	strcpy(intcbuf,argv1);
	strcat(intcbuf,argv2);
	sprintf(argv1,",%02X",b1);
	strcat(intcbuf,argv1);

    return;
}

void InterpretMODSREGRM(char *insname)
{
	char argv1[32],argv2[32];
	char c;
	WORD w1;
	BYTE b1;

	argv1[0] = argv2[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
			sprintf(argv1,"WORD PTR %s[%04Xh]",segment_prefix_dec,w1);
			sprintf(argv2,"%s",regsegs[fl_reg]);
		}
		else {
			sprintf(argv1,"WORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
			sprintf(argv2,"%s",regsegs[fl_reg]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';
		sprintf(argv1,"WORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
		sprintf(argv2,"%s",regsegs[fl_reg]);
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		c = (w1 >= 0x8000) ? '-' : '+';
		sprintf(argv1,"WORD PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],c,abs((short int)w1));
		sprintf(argv2,"%s",regsegs[fl_reg]);
	}
	if (fl_mod == 3) {
		sprintf(argv1,"%s",regs16[fl_rm]);
		sprintf(argv2,"%s",regsegs[fl_reg]);
	}

	if (fl_d)	sprintf(intcbuf,"%-7s %s,%s",insname,argv2,argv1);
	else		sprintf(intcbuf,"%-7s %s,%s",insname,argv1,argv2);

    return;
}

void InterpretMODLIRM(char *insname)
{
	char argv1[32],argv2[32];
	char c;
    WORD w1;
	BYTE b1;

	argv1[0] = argv2[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w1  = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
			sprintf(argv1,"DWORD PTR %s[%04Xh]",segment_prefix_dec,w1);
			sprintf(argv2,"%s",regs16[fl_reg]);
		}
		else {
			sprintf(argv1,"DWORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
			sprintf(argv2,"%s",regs16[fl_reg]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';
		sprintf(argv1,"DWORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
		sprintf(argv2,"%s",regs16[fl_reg]);
	}
	if (fl_mod == 2) {
		w1  = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		c = (w1 >= 0x8000) ? '-' : '+';
		sprintf(argv1,"DWORD PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],c,abs((short int)w1));
		sprintf(argv2,"%s",regs16[fl_reg]);
	}

	if (fl_mod == 3) {
		sprintf(intcbuf,"???");
	}
	else {
		sprintf(intcbuf,"%-7s %s,%s",insname,argv2,argv1);
	}

    return;
}

void InterpretMODLEARM(char *insname)
{
	char argv1[32],argv2[32];
	char c;
	WORD w1;
	BYTE b1;

	argv1[0] = argv2[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
			sprintf(argv1,"%s[%04Xh]",segment_prefix_dec,w1);
			sprintf(argv2,"%s",regs16[fl_reg]);
		}
		else {
			sprintf(argv1,"%s[%s]",segment_prefix_dec,displ[fl_rm]);
			sprintf(argv2,"%s",regs16[fl_reg]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';
		sprintf(argv1,"%s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
		sprintf(argv2,"%s",regs16[fl_reg]);
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		c = (w1 >= 0x8000) ? '-' : '+';
		sprintf(argv1,"%s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],c,abs((short int)w1));
		sprintf(argv2,"%s",regs16[fl_reg]);
	}

	if (fl_mod == 3) {
		sprintf(intcbuf,"???");
	}
	else {
		sprintf(intcbuf,"%-7s %s,%s",insname,argv2,argv1);
	}

    return;
}

void InterpretMOD010RM(char *insname)
{
	char argv1[32],argv2[32];
	char c,c2;
	WORD w1,w2;
	BYTE b1,b2;

	if (db67 && fl_mod != 3 && fl_rm == 4) {
		sprintf(intcbuf,"%s [SIB bytes unsupported]",insname);
		fl_bytes = 0;
        return;
	}

	argv1[0] = argv2[0] = 0;

	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;

			if (fl_w) {
				if (fl_s) {
					sprintf(argv1,"WORD PTR %s[%04Xh]",segment_prefix_dec,w1);
					b1 = CPUDecoderQueueFetch();
					c = (b1 >= 0x80) ? '-' : '+';
					sprintf(argv2,"%c%02Xh",c,abs((char)b1));
				}
				else {
					w2 = (WORD)CPUDecoderQueueFetch();
					w2 |= ((WORD)CPUDecoderQueueFetch())<<8;

					sprintf(argv1,"WORD PTR %s[%04Xh]",segment_prefix_dec,w1);
					sprintf(argv2,"%04Xh",w2);
				}
			}
			else {
				b1 = CPUDecoderQueueFetch();
				sprintf(argv1,"BYTE PTR %s[%04Xh]",segment_prefix_dec,w1);
				sprintf(argv2,"%02Xh",b1);
			}
		}
		else {
			if (fl_w) {
				if (fl_s) {
					sprintf(argv1,"WORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
					b1 = CPUDecoderQueueFetch();
					c = (b1 >= 0x80) ? '-' : '+';
					sprintf(argv2,"%c%02Xh",c,abs((char)b1));
				}
				else {
					w2 = (WORD)CPUDecoderQueueFetch();
					w2 |= ((WORD)CPUDecoderQueueFetch())<<8;
					sprintf(argv1,"WORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
					sprintf(argv2,"%04Xh",w2);
				}
			}
			else {
				b1 = CPUDecoderQueueFetch();
				sprintf(argv1,"BYTE PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
				sprintf(argv2,"%02Xh",b1);
			}
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';

		if (fl_w) {
			if (fl_s) {
				b2 = CPUDecoderQueueFetch();
				c2 = (b2 >= 0x80) ? '-' : '+';
				sprintf(argv1,"WORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c2,abs((char)b1));
				sprintf(argv2,"%c%02Xh",c,abs((char)b2));
			}
			else {
				w2 = (WORD)CPUDecoderQueueFetch();
				w2 |= ((WORD)CPUDecoderQueueFetch())<<8;
				sprintf(argv1,"WORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
				sprintf(argv2,"%04Xh",w2);
			}
		}
		else {
			b2 = CPUDecoderQueueFetch();
			sprintf(argv1,"BYTE PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
			sprintf(argv2,"%02Xh",b2);
		}
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;

		if (fl_w) {
			if (fl_s) {
				b2 = CPUDecoderQueueFetch();
				c = (b2 >= 0x80) ? '-' : '+';
				sprintf(argv1,"WORD PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],'+',w1);
				sprintf(argv2,"%c%02Xh",c,abs((char)b2));
			}
			else {
				w2 = (WORD)CPUDecoderQueueFetch();
				w2 |= ((WORD)CPUDecoderQueueFetch())<<8;
				sprintf(argv1,"WORD PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],'+',w1);
				sprintf(argv2,"%04Xh",w2);
			}
		}
		else {
			b2 = CPUDecoderQueueFetch();
			sprintf(argv1,"BYTE PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],'+',w1);
			sprintf(argv2,"%02Xh",b2);
		}
	}
	if (fl_mod == 3) {
		if (fl_w) {
			if (fl_s) {
				b2 = CPUDecoderQueueFetch();
				sprintf(argv1,"%s",regs16[fl_rm]);
				if (b2 >= 0x80)		sprintf(argv2,"-%02Xh",0x100-b2);
				else				sprintf(argv2,"+%02Xh",b2);
			}
			else {
				w2 = (WORD)CPUDecoderQueueFetch();
				w2 |= ((WORD)CPUDecoderQueueFetch())<<8;
				sprintf(argv1,"%s",regs16[fl_rm]);
				sprintf(argv2,"%04Xh",w2);
			}
		}
		else {
			b2 = CPUDecoderQueueFetch();
			sprintf(argv1,"%s",regs8[fl_rm]);
			sprintf(argv2,"%02Xh",b2);
		}
	}

	sprintf(intcbuf,"%-7s %s,%s",insname,argv1,argv2);

    return;
}

void InterpretMODS1RM(char *insname)
{
	char argv1[32];
	char c;
	WORD w1;
	BYTE b1;

	argv1[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch()) << 8;

			if (fl_w)		sprintf(argv1,"WORD PTR %s[%04Xh]",segment_prefix_dec,w1);
			else			sprintf(argv1,"BYTE PTR %s[%04Xh]",segment_prefix_dec,w1);
		}
		else {
			if (fl_w)		sprintf(argv1,"WORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
			else			sprintf(argv1,"BYTE PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';

		if (fl_w)		sprintf(argv1,"WORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
		else			sprintf(argv1,"BYTE PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch()) << 8;
		c = (w1 >= 0x8000) ? '-' : '+';

		if (fl_w)		sprintf(argv1,"WORD PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],c,abs((short int)w1));
		else			sprintf(argv1,"BYTE PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],c,abs((short int)w1));
	}
	if (fl_mod == 3) {
		if (fl_w)		sprintf(argv1,"%s",regs16[fl_rm]);
		else			sprintf(argv1,"%s",regs8[fl_rm]);
	}

	sprintf(intcbuf,"%-7s %s",insname,argv1);

    return;
}

void InterpretMODS1RM2(char *insname)
{
	char argv1[32];
	char c;
    DWORD o=0,o2=0;

	argv1[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			sprintf(argv1,"[%04Xh]",memwordlinear(o)); o += 2;
		}
		else {
			sprintf(argv1,"[%s]",displ[fl_rm]);
		}
	}
	if (fl_mod == 1) {
		c = '+';
		if (membytelinear(o) >= 0x80) c = '-';
		sprintf(argv1,"[%s%c%02Xh]",displ[fl_rm],c,abs((char)membytelinear(o))); o++;
	}
	if (fl_mod == 2) {
		c = '+';
		if (memwordlinear(o) >= 0x8000) c = '-';
		sprintf(argv1,"[%s%c%04Xh]",displ[fl_rm],c,abs((short int)memwordlinear(o))); o += 2;
	}
	if (fl_mod == 3) {
		sprintf(argv1,"%s",regs16[fl_rm]);
	}
	sprintf(intcbuf,"%-7s %s%s",insname,fl_prefix,argv1);
	fl_bytes = o-o2;
    return;
}

void InterpretMODSCLRM(char *insname)
{
	char argv1[32];
	char c;
	WORD w1;
	BYTE b1;

	argv1[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
			if (fl_w)	sprintf(argv1,"WORD PTR %s[%04Xh]",segment_prefix_dec,w1);
			else		sprintf(argv1,"BYTE PTR %s[%04Xh]",segment_prefix_dec,w1);
		}
		else {
			if (fl_w)	sprintf(argv1,"WORD PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
			else		sprintf(argv1,"BYTE PTR %s[%s]",segment_prefix_dec,displ[fl_rm]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';
		if (fl_w)	sprintf(argv1,"WORD PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
		else		sprintf(argv1,"BYTE PTR %s[%s%c%02Xh]",segment_prefix_dec,displ[fl_rm],c,abs((char)b1));
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		c = (w1 >= 0x8000) ? '-' : '+';
		if (fl_w)	sprintf(argv1,"WORD PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],c,abs((short int)w1));
		else		sprintf(argv1,"BYTE PTR %s[%s%c%04Xh]",segment_prefix_dec,displ[fl_rm],c,abs((short int)w1));
	}
	if (fl_mod == 3) {
		if (fl_w)	sprintf(argv1,"%s",regs16[fl_rm]);
		else		sprintf(argv1,"%s",regs8[fl_rm]);
	}

	sprintf(intcbuf,"%-7s %s%s,CL",insname,fl_prefix,argv1);

    return;
}

void InterpretMODS1BRM(char *insname)
{
	char argv1[32];
	char c;
	WORD w1;
	BYTE b1;

	argv1[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			w1 = (WORD)CPUDecoderQueueFetch();
			w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
			if (fl_w)	sprintf(argv1,"WORD PTR [%04Xh]",w1);
			else		sprintf(argv1,"BYTE PTR [%04Xh]",w1);
		}
		else {
			if (fl_w)	sprintf(argv1,"WORD PTR [%s]",displ[fl_rm]);
			else		sprintf(argv1,"BYTE PTR [%s]",displ[fl_rm]);
		}
	}
	if (fl_mod == 1) {
		b1 = CPUDecoderQueueFetch();
		c = (b1 >= 0x80) ? '-' : '+';
		if (fl_w)	sprintf(argv1,"WORD PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)b1));
		else		sprintf(argv1,"BYTE PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)b1));
	}
	if (fl_mod == 2) {
		w1 = (WORD)CPUDecoderQueueFetch();
		w1 |= ((WORD)CPUDecoderQueueFetch())<<8;
		c = (w1 >= 0x8000) ? '-' : '+';
		if (fl_w)	sprintf(argv1,"WORD PTR [%s%c%04Xh]",displ[fl_rm],c,abs((short int)w1));
		else		sprintf(argv1,"BYTE PTR [%s%c%04Xh]",displ[fl_rm],c,abs((short int)w1));
	}
	if (fl_mod == 3) {
		if (fl_w)	sprintf(argv1,"%s",regs16[fl_rm]);
		else		sprintf(argv1,"%s",regs8[fl_rm]);
	}

	sprintf(intcbuf,"%-7s %s,1",insname,argv1);

    return;
}

void InterpretMODSHIRM(char *insname)
{
	char argv1[32];
	char c;
    DWORD o=0,o2=0;

	argv1[0] = 0;
	if (fl_mod == 0) {
		if (fl_rm == 6) {
			if (fl_w) {
				if (db67) {
					if (db66)	sprintf(argv1,"DWORD PTR [%08Xh]",memdwordlinear(o));
					else		sprintf(argv1,"WORD PTR [%08Xh]",memdwordlinear(o));
					o += 4;
				}
				else {
					if (db66)	sprintf(argv1,"DWORD PTR [%04Xh]",memwordlinear(o));
					else		sprintf(argv1,"WORD PTR [%04Xh]",memwordlinear(o));
					o += 2;
				}
			}
			else {
				if (db67) {
					sprintf(argv1,"BYTE PTR [%08Xh]",memdwordlinear(o));
					o += 4;
				}
				else {
					sprintf(argv1,"BYTE PTR [%04Xh]",memwordlinear(o));
					o += 2;
				}
			}
		}
		else {
			if (fl_w) {
				if (db67) {
					if (db66)	sprintf(argv1,"DWORD PTR [%s]",displ32[fl_rm]);
					else		sprintf(argv1,"WORD PTR [%s]",displ32[fl_rm]);
				}
				else {
					if (db66)	sprintf(argv1,"DWORD PTR [%s]",displ[fl_rm]);
					else		sprintf(argv1,"WORD PTR [%s]",displ[fl_rm]);
				}
			}
			else {
				if (db67)
					sprintf(argv1,"BYTE PTR [%s]",displ32[fl_rm]);
				else
					sprintf(argv1,"BYTE PTR [%s]",displ[fl_rm]);
			}
		}
	}
	if (fl_mod == 1) {
		c = '+';
		if (membytelinear(o) >= 0x80) c = '-';
		if (fl_w) {
			if (db67) {
				if (db66)	sprintf(argv1,"DWORD PTR [%s%c%02Xh]",displ32[fl_rm],c,abs((char)membytelinear(o++)));
				else		sprintf(argv1,"WORD PTR [%s%c%02Xh]",displ32[fl_rm],c,abs((char)membytelinear(o++)));
			}
			else {
				if (db66)	sprintf(argv1,"DWORD PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)membytelinear(o++)));
				else		sprintf(argv1,"WORD PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)membytelinear(o++)));
			}
		}
		else {
			sprintf(argv1,"BYTE PTR [%s%c%02Xh]",displ[fl_rm],c,abs((char)membytelinear(o)));	o++;
		}
	}
	if (fl_mod == 2) {
		if (db67) {
			c = '+';
			if (memdwordlinear(o) >= 0x80000000) c = '-';
			if (fl_w) {
				if (db66)	sprintf(argv1,"DWORD PTR [%s%c%08Xh]",displ32[fl_rm],c,abs((int)memdwordlinear(o)));
				else		sprintf(argv1,"WORD PTR [%s%c%08Xh]",displ32[fl_rm],c,abs((int)memdwordlinear(o)));
			}
			else {
				sprintf(argv1,"BYTE PTR [%s%c%08Xh]",displ[fl_rm],c,abs((int)memdwordlinear(o)));
			}
			o += 4;
		}
		else {
			c = '+';
			if (memwordlinear(o) >= 0x8000) c = '-';
			if (fl_w) {
				if (db66)	sprintf(argv1,"DWORD PTR [%s%c%04Xh]",displ[fl_rm],c,abs((short int)memwordlinear(o)));
				else		sprintf(argv1,"WORD PTR [%s%c%04Xh]",displ[fl_rm],c,abs((short int)memwordlinear(o)));
			}
			else {
				sprintf(argv1,"BYTE PTR [%s%c%04Xh]",displ[fl_rm],c,abs((short int)memwordlinear(o)));
			}
			o += 2;
		}
	}
	if (fl_mod == 3) {
		if (fl_w) {
			if (db66)	sprintf(argv1,"%s",regs32[fl_rm]);
			else		sprintf(argv1,"%s",regs16[fl_rm]);
		}
		else {
			sprintf(argv1,"%s",regs8[fl_rm]);
		}
	}
	sprintf(intcbuf,"%-7s %s%s,%02Xh",insname,fl_prefix,argv1,membytelinear(o)); o++;
	fl_bytes = o-o2;
	db66a = db67a = 1;
    return;
}
