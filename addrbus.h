// header file for memory.cpp

#define AB_MOD_READ						1		// IO read
#define AB_MOD_WRITE					2		// IO write
#define AB_MOD_SNAPSHOT					3		// snapshot read
#define AB_MOD_SNAPSHOTWRITE			4		// snapshot write

#define ADDRBUSARRAYLEN	(1<<20)

void AddressBusEmuInit();
void AddressBusEmuClose();
void AddressBusReset();
void AddressBusEmuPowerOn();
void AddressBusEmuPowerOff();
int IsAddressBusPowered();
// NOTE: FOR VIRTUAL CPU USE ONLY (BECAUSE THESE ROUTINES TRANSLATE ADDRESSES)
void writemembytefarptr(DWORD seg,DWORD offs,BYTE v);
void writemembytelinear(DWORD o,BYTE v);
void writememwordfarptr(DWORD seg,DWORD offs,WORD w);
void writememwordlinear(DWORD o,WORD w);
void writememdwordfarptr(DWORD seg,DWORD offs,DWORD w);
void writememdwordlinear(DWORD o,DWORD d);
BYTE membytefarptr(DWORD seg,DWORD offs);
BYTE membytelinear(DWORD o);
WORD memwordfarptr(DWORD seg,DWORD offs);
WORD memwordlinear(DWORD o);
DWORD memdwordfarptr(DWORD seg,DWORD offs);
DWORD memdwordlinear(DWORD o);
// NOTE: FOR HARDWARE EMULATION PURPOSES
void hardwritemembyte(DWORD o,BYTE v);
void hardwritememword(DWORD o,WORD w);
void hardwritememdword(DWORD o,DWORD d);
BYTE hardmembyte(DWORD o);
WORD hardmemword(DWORD o);
DWORD hardmemdword(DWORD o);

int addrbus_make_page_empty(DWORD base);
int addrbus_make_page_adaptor(DWORD base,DWORD (*mem_call)(int size,int mod,DWORD dat,DWORD addr));

extern BOOL						addrbus_a20;
