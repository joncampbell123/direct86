// CMOS/RTC clock simulator

#include "global.h"
#include "addrbus.h"
#include "hardware.h"
#include "direct86.h"
#include "cpuexec.h"
#include "lib.h"
#include "cmos.h"

BOOL				RTCBatteryDead=FALSE;
BYTE				CMOSmemory[128];
DWORD				RTC_basetime=0,RTC_dtime;
DWORD				RTC_reltime;
SYSTEMTIME			RTC_time;
int					CMOS_idle_wait=0;
int					CMOSIndex=0;
int					CMOS_power=0;

DWORD cmos_70h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)CMOSIndex;
	}

	if (dir == IO_PORT_OUT) {
		CMOSIndex = (int)(data&0xFF);
	}

	return r;
}

DWORD cmos_71h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		r = (DWORD)CMOSmemory[CMOSIndex];
	}

	if (dir == IO_PORT_OUT) {
		CMOSmemory[CMOSIndex] = (BYTE)(data&0xFF);
	}

	return r;
}

void CMOSInit()
{
	CMOS_power=0;
}

void CMOSClose()
{
	CMOS_power=0;
	unassign_IO_port(0x70);
	unassign_IO_port(0x71);
}

void CMOSReset()
{
	if (!CMOS_power) return;

	assign_IO_port(0x70,cmos_70h);
	assign_IO_port(0x71,cmos_71h);
}

void CMOSCycle()
{
	BYTE t;

	if (!CMOS_power) return;

// have an idle wait because GetSystemTime() wastes a lot of time
	CMOS_idle_wait--;
	if (CMOS_idle_wait <= 0 || !code_freerun) {
		CMOS_idle_wait = 1024;

		RTC_dtime = (DWORD)frtime(NULL);
		if (RTC_basetime == 0) {
			RTC_basetime = RTC_dtime;
			RTC_reltime = 0;
		}
		RTC_dtime -= RTC_basetime;
		RTC_dtime += RTC_reltime;

// get the "time" (timed using the system clock and then recalculated
// to the simulated clock's time)
		RTC_time = timefr(RTC_dtime);
// put it in the CMOS
		CMOSmemory[0] = (BYTE)DecToPackedBCD((DWORD)RTC_time.wSecond);
		CMOSmemory[2] = (BYTE)DecToPackedBCD((DWORD)RTC_time.wMinute);
		CMOSmemory[4] = (BYTE)DecToPackedBCD((DWORD)RTC_time.wHour);
		CMOSmemory[6] = (BYTE)DecToPackedBCD((DWORD)RTC_time.wDayOfWeek);
		CMOSmemory[7] = (BYTE)DecToPackedBCD((DWORD)RTC_time.wDay);
		CMOSmemory[8] = (BYTE)DecToPackedBCD((DWORD)RTC_time.wMonth);
// I've noticed that many RTC's store the year in packed BCD format but
// allow the second digit to increase beyond 0Ah to represent years past
// 2000. So, that's the way we'll do it
		t = (BYTE)((RTC_time.wYear - 1900) % 10);
		t |= (BYTE)(((RTC_time.wYear - 1900) / 10)<<4);
		CMOSmemory[9] = t;
	}
}

void CMOSPowerDown()
{
	CMOS_power=0;
	CMOSClose();
}

void CMOSPowerUp()
{
	CMOS_power=1;
	CMOSReset();
}

int CMOSIsPowered()
{
	return CMOS_power;
}
