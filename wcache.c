
#include "global.h"
#include "addrbus.h"
#include "wcache.h"
#include "wcachew.h"

int					wmem_update;

// code for CPU write-through cache emulator

void cpu_writethru_cache_init()
{
}

void cpu_writethru_cache_free()
{
}

void cpu_writethru_cache_dump()
{
	WCRepaint=TRUE;
}

void cpu_writethru_cache_setsize(int size)
{
	cpu_writethru_cache_dump();
	WCRepaint=TRUE;
}

void cpu_writethru_cache_write(DWORD o,BYTE dat)
{
	hardwritemembyte(o,dat);
}

BYTE cpu_writethru_cache_read(DWORD o)
{
	return hardmembyte(o);
}
