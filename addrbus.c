
#include "global.h"
#include "wcache.h"
#include "cpuexec.h"
#include "memwin.h"
#include "ram.h"
#include "brkpts.h"
#include "addrbus.h"
#include "direct86.h"

// NOTE: THIS IS NOW THE FRAMEWORK FOR THE ADDRESS BUS.
//       RAM EMULATION IS IN ANOTHER SOURCE FILE.
DWORD				addrbus_access_empty(int size,int mod,DWORD dat,DWORD addr);
// memory is organized into pages (4K chunks), the array being large enough that it can fullfill the entire 4GB range
DWORD				(**addrbus_access_call)(int size,int mod,DWORD dat,DWORD addr)=NULL;
BOOL				addrbus_a20=TRUE;
int					addrbus_power=0;

void AddressBusEmuInit()
{
	int x;
	char *ptr=NULL;		// used as a ref for sizeof()

	addrbus_access_call = (DWORD (**)(int,int,DWORD,DWORD))malloc(ADDRBUSARRAYLEN*sizeof(ptr));
	if (!addrbus_access_call) {
		MessageBox(hwndMain,"ADDRESS BUS EMULATION ERR: Unable to allocate fake_memory callback array","Address bus error",MB_OK);
		return;
	}

	for (x=0;x < ADDRBUSARRAYLEN;x++) addrbus_access_call[x] = addrbus_access_empty;

	addrbus_power=0;
	addrbus_a20=TRUE;
}

void AddressBusEmuClose()
{
	int x;

	if (!addrbus_access_call) return;
	for (x=0;x < ADDRBUSARRAYLEN;x++) addrbus_access_call[x]=NULL;
// delete memory associated with pointer array.
// addrbus_access_call was pointed to share that array, so no need to delete it too.
	if (addrbus_access_call) free(addrbus_access_call);
	addrbus_access_call=NULL;
}

void AddressBusReset()
{
}

int IsAddressBusPowered()
{
	return addrbus_power;
}

void AddressBusEmuPowerOn()
{
	addrbus_power=1;
	MemoryWinUpdate=TRUE;
}

void AddressBusEmuPowerOff()
{
	addrbus_power=0;
	MemoryWinUpdate=TRUE;
}

// NOTE: FOR VIRTUAL CPU USE ONLY (BECAUSE THESE ROUTINES TRANSLATE ADDRESSES)

BYTE membytelinear(DWORD o)
{
	CheckTriggerMemBreakpoint(o,1,BPTR_READ);
	return hardmembyte(o);
}

void writemembytelinear(DWORD o,BYTE v)
{
	CheckTriggerMemBreakpoint(o,1,BPTR_WRITE);
	hardwritemembyte(o,v);
}

WORD memwordlinear(DWORD o)
{
	CheckTriggerMemBreakpoint(o,2,BPTR_READ);
	return hardmemword(o);
}

void writememwordlinear(DWORD o,WORD w)
{
	CheckTriggerMemBreakpoint(o,2,BPTR_WRITE);
	hardwritememword(o,w);
}

DWORD memdwordlinear(DWORD o)
{
	CheckTriggerMemBreakpoint(o,4,BPTR_READ);
	return hardmemdword(o);
}

void writememdwordlinear(DWORD o,DWORD d)
{
	CheckTriggerMemBreakpoint(o,4,BPTR_WRITE);
	hardwritememdword(o,d);
}

BYTE membytefarptr(DWORD seg,DWORD offs)
{
	return membytelinear((seg << 4) + offs);
}

void writemembytefarptr(DWORD seg,DWORD offs,BYTE v)
{
	writemembytelinear((seg << 4) + offs,v);
}

WORD memwordfarptr(DWORD seg,DWORD offs)
{
	return memwordlinear((seg << 4) + offs);
}

void writememwordfarptr(DWORD seg,DWORD offs,WORD w)
{
	writememwordlinear((seg << 4) + offs,w);
}

DWORD memdwordfarptr(DWORD seg,DWORD offs)
{
	return memdwordlinear((seg << 4) + offs);
}

void writememdwordfarptr(DWORD seg,DWORD offs,DWORD w)
{
	writememdwordlinear((seg << 4) + offs,w);
}

// NOTE: FOR HARDWARE EMULATION PURPOSES

BYTE hardmembyte(DWORD o)
{
	if (!addrbus_access_call) return 0xFF;
	if (!addrbus_power) return 0xFF;

	return (BYTE)addrbus_access_call[o>>12](1,AB_MOD_READ,0,o);
}

BYTE snapshothardmembyte(DWORD o)
{
	if (!addrbus_access_call) return 0xFF;
	if (!addrbus_power) return 0xFF;

	return (BYTE)addrbus_access_call[o>>12](1,AB_MOD_SNAPSHOT,0,o);
}

void hardwritemembyte(DWORD o,BYTE v)
{
	BOOL ShowRepaint=FALSE;

	if (!addrbus_access_call) return;
	if (!addrbus_power) return;

	addrbus_access_call[o>>12](1,AB_MOD_WRITE,(DWORD)v,o);

// make sure changes show up in memory window
	ShowRepaint = (o >= memory_win_begin && o <= memory_win_end);
	if (code_freerun) {
		wmem_update--;
		if (wmem_update <= 0) {
			if (code_showallsteps)
				wmem_update=40;
			else
				wmem_update=5000;
		}
		else {
			ShowRepaint = FALSE;
		}
	}
	if (ShowRepaint)
		MemoryWinUpdate=TRUE;
}

WORD hardmemword(DWORD o)
{
	return (WORD)(((DWORD)hardmembyte(o)) | (((DWORD)hardmembyte(o+1))<<8));
}

void hardwritememword(DWORD o,WORD w)
{
	hardwritemembyte(o,(BYTE)(w&0xFF));
	hardwritemembyte(o+1,(BYTE)((w>>8)&0xFF));
}

DWORD hardmemdword(DWORD o)
{
	return ((DWORD)hardmembyte(o)) | (((DWORD)hardmembyte(o+1))<<8) | (((DWORD)hardmembyte(o+2))<<16) | (((DWORD)hardmembyte(o+3))<<24);
}

void hardwritememdword(DWORD o,DWORD d)
{
	hardwritemembyte(o,(BYTE)(d&0xFF));
	hardwritemembyte(o+1,(BYTE)((d>>8)&0xFF));
	hardwritemembyte(o+2,(BYTE)((d>>16)&0xFF));
	hardwritemembyte(o+3,(BYTE)((d>>24)&0xFF));
}

// default RAM access handler for empty areas
DWORD addrbus_access_empty(int size,int mod,DWORD dat,DWORD addr)
{
	return 0xFFFFFFFF;
}

// API for other parts of this program to set up adapter RAM and
// stuff

int addrbus_make_page_empty(DWORD base)
{
	BOOL ShowRepaint=FALSE;

	if (!addrbus_access_call) return 0;
// must be aligned to a 4K buffer
	if (base&0xFFF) return 0;

// make sure changes show up in memory window
	ShowRepaint = (base >= (memory_win_begin & ~0xFFF) && base <= ((memory_win_end & ~0xFFF)+4095));
	if (code_freerun) {
		wmem_update--;
		if (wmem_update <= 0) {
			if (code_showallsteps)
				wmem_update=40;
			else
				wmem_update=500;
		}
		else {
			ShowRepaint = FALSE;
		}
	}

	if (ShowRepaint) MemoryWinUpdate=TRUE;

	base >>= 12;
	addrbus_access_call[base] = addrbus_access_empty;

	return 1;
}

int addrbus_make_page_adaptor(DWORD base,DWORD (*ab_call)(int size,int mod,DWORD dat,DWORD addr))
{
	BOOL ShowRepaint=FALSE;

	if (!addrbus_access_call) return 0;
// must be aligned to a 4K buffer
	if (base&0xFFF) return 0;

// make sure changes show up in memory window
	ShowRepaint = (base >= (memory_win_begin & ~0xFFF) && base <= ((memory_win_end & ~0xFFF)+4095));
	if (code_freerun) {
		wmem_update--;
		if (wmem_update <= 0) {
			if (code_showallsteps)
				wmem_update=40;
			else
				wmem_update=500;
		}
		else {
			ShowRepaint = FALSE;
		}
	}

	if (ShowRepaint) MemoryWinUpdate=TRUE;

	base >>= 12;
	addrbus_access_call[base] = ab_call;

	return 1;
}
