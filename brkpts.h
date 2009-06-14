
#define MAX_BREAKPOINTS		1024

#define BPT_NONE			0		// no breakpoint
#define BPT_MEMRANGE		1		// range of memory
#define BPT_IOPORTRANGE		2		// IO port range
#define BPT_SWINTRANGE		3		// software interrupt range
#define BPT_HWINTRANGE		4		// hardware interrupt range

#define BPTR_READ			0x00000001	// trigger on read
#define BPTR_WRITE			0x00000002	// trigger on write

typedef struct {
	int		type;						// type of breakpoint
	int		trigger;					// trigger flags
	char	enabled;					// enabled
	DWORD	mem_addr,mem_addr_end;		// memory trigger vars
	DWORD	io_addr,io_addr_end;		// IO port trigger vars
	int		swint_addr,swint_addr_end;	// S/W int trigger vars
	int		hwint_addr,hwint_addr_end;	// H/W int trigger vars
} BREAKPOINT;

extern int							hold_breakpoint_triggers;
extern int							exec_show_breakpoint;

void BreakpointTriggerCease(int i);
int DoesTriggerMemBreakpoint(DWORD addr,DWORD siz,int access);
void CheckTriggerMemBreakpoint(DWORD addr,DWORD siz,int access);
int DoesTriggerIOBreakpoint(DWORD addr,DWORD siz,int access);
void CheckTriggerIOBreakpoint(DWORD addr,DWORD siz,int access);
int DoesTriggerSWINTBreakpoint(int j);
void CheckTriggerSWINTBreakpoint(int j);
int DoesTriggerHWINTBreakpoint(int j);
void CheckTriggerHWINTBreakpoint(int j);
void SignalBreakPointTrigger(int idx);
void RemoveBreakPoint(int x);
void PackBreakPoints();
void UserEditBreakPoints();

void CloseBreakPointSys();
void InitBreakPointSys();
void PowerOnBreakPointSys();
void PowerOffBreakPointSys();
int IsBreakPointSysPowered();
void ResetBreakPointSys();
