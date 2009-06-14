
void CPUStackWatchInit();
void CPUStackWatchClose();
void CPUStackWatchPowerOn();
void CPUStackWatchPowerOff();
void CPUStackWatchReset();
int IsCPUStackWatchPowered();

typedef struct {
	int			type;			/* type of stack operation */
	DWORD		cs,ip;			/* resulting address (means same as from_cs:from_ip unless type respresents call stacks) */
	DWORD		from_cs,from_ip;/* address of stack operation */
	DWORD		old_sp,new_sp;	/* old and new values of (E)SP */
	DWORD		old_ss,new_ss;	/* old and new values of SS */
	int			intnum;			/* number of interrupt (if interrupt related) */
} STACKWOP;
#define MAXSTACKWOPS		10000

extern STACKWOP					stack_watch_ops[MAXSTACKWOPS];
extern int						stack_watch_ops_r,stack_watch_ops_w;
extern int						cpu_stack_power;

#define SWT_EMPTY				0		/* entry space for rent */
#define SWT_CALLNEAR			1		/* entry represents near (intrasegment) call stack frame generated */
#define SWT_CALLFAR				2		/* entry represents far (intersegment) call stack frame generated */
#define SWT_CALLINT				3		/* entry represents interrupt stack frame generated */
#define SWT_DISREGARD			4		/* entry represents total disregard for stack pointer (i.e., loading completely new values into SS or SP) */
#define SWT_RET					5		/* entry represents use and discarding of near (intrasegment) call stack frame (i.e. RET) */
#define SWT_RETF				6		/* entry represents use and discarding of far (intersegment) call stack frame (i.e. RETF) */
#define SWT_IRET				7		/* entry represents use and discarding of interrupt stack frame (i.e. IRET) */
#define SWT_PUSH				8		/* entry represents storage of data on the stack */
#define SWT_POP					9		/* entry represents discarding of data on the stack */
#define SWT_PUSHALLOC			10		/* entry represents allocation of stack memory by adjusting SP (i.e., SUB SP,52) */
#define SWT_POPIGNORE			11		/* entry represents complete ignorance of discarding data (i.e., CALL FAR 445, ADD SP,2, RET) */
#define SWT_HWCALLINT			12		/* entry represents interrupt stack frame generated (H/W interrupt) */

void ClearStackWatch();
void AddStackWatchEntry(int type,DWORD cs,DWORD ip,DWORD ocs,DWORD oip,DWORD oss,DWORD nss,DWORD osp,DWORD nsp);
void AddStackWatchEntryINT(int type,DWORD cs,DWORD ip,DWORD ocs,DWORD oip,DWORD oss,DWORD nss,DWORD osp,DWORD nsp,int intnum);
void StackWatchView();
