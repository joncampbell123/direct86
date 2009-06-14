// CPU instruction queue emulation
// basically, all this code provides a way to sequentially
// fetch an opcode at a time regardless

#include "global.h"
#include "direct86.h"
#include "cpuexec.h"
#include "cpuqueue.h"
#include "stackops.h"
#include "addrbus.h"
#include "naming.h"

DWORD deci_eip;
DWORD deci_cs;
int deci_is_decoding=0;

void FlushCPUQueue()
{
// do nothing, yet
}

BYTE CPUQueueFetch()
{
	BYTE b;

	if (deci_is_decoding) {
		code_freerun=0;
		MessageBox(hwndMain,"WARNING: CPUQueueFetch() called and decoder queue active","",MB_OK);
	}

	b = membytefarptr(ireg_cs,ireg_eip++);

	if (naming_enabled) MatchNamingSeq(b,ireg_cs,ireg_eip-1);

	return b;
}

// for the decoder

void BeginDecoderQueue(DWORD seg,DWORD ofs)
{
	deci_eip = ofs;
	deci_cs  = seg;
	deci_is_decoding = 1;
}

void FlushCPUDecoderQueue()
{
}

BYTE CPUDecoderQueueFetch()
{
	BYTE b;

	if (!deci_is_decoding) {
		code_freerun=0;
		MessageBox(hwndMain,"WARNING: CPUDecoderQueueFetch() called and decoder queue not ready","",MB_OK);
	}

	b = membytefarptr(deci_cs,deci_eip++);

	return b;
}

void EndDecoderQueue(DWORD *seg,DWORD *ofs)
{
	if (ofs)	*ofs = deci_eip;
	if (seg)	*seg = deci_cs;
	deci_is_decoding = 0;
}
