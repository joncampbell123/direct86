
// list of previous locations of the CPU. Circular buffer so
// updating is not a slow hassle
typedef struct {
	BOOL		valid;
	DWORD		ip,cs;
	DWORD		rep;
} CPULOC;

#define CPULOCMAX		4096

void AddPrevCPUAddr(DWORD cs,DWORD ip);
void CPUTrailsView();
void CPUTrailsInit();
void CPUTrailsClose();
void CPUTrailsPowerOn();
void CPUTrailsPowerOff();
void CPUTrailsReset();
int IsCPUTrailsPowered();
