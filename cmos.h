
extern BOOL					RTCBatteryDead;
extern BYTE					CMOSmemory[128];
extern DWORD				RTC_basetime,RTC_dtime;
extern DWORD				RTC_reltime;
extern SYSTEMTIME			RTC_time;
extern int					CMOS_idle_wait;
extern int					CMOSIndex;

void CMOSInit();
void CMOSClose();
void CMOSReset();
void CMOSCycle();
void CMOSPowerDown();
void CMOSPowerUp();
int CMOSIsPowered();
