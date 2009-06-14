
typedef struct {
	int			int_base;			// interrupt # base
	int			base_io;			// base I/O port
	int			int_mask;			// interrupt mask
	int			int_pending;		// interrupt pending signals
	int			int_inservice;		// interrupt in-serivce signals
	int			int_lowest_prio;	// interrupt with lowest priority
	int			last_int;			// last interrupt that occured (used for non-specific EOI emulation)
	int			cmddat_ret;			// data to return through command port
	int			mode;				// mode of controller
	int			irq_base;			// base of IRQ #
} pic_controller;

#define PIC_MODE_NORMAL			0	// normal mode
#define PIC_MODE_ISRRET			1	// ISR returning mode
#define PIC_MODE_IRRRET			2	// IRR returning mode

extern char*						szClassNamePIC;
extern HDC							PICWinDC;
extern int							PICWindowRect_w,PICWindowRect_h;
extern BOOL							PICRepaint;
extern HWND							hwndMainPIC;

void HW_Signal_IRQ(int n);
void PICCycle();
void PICReset();
void InitPIC();
void ClosePIC();
void PICPowerOn();
void PICPowerOff();
int PICIsPowered();

long FAR PASCAL MainWndProcPIC(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
