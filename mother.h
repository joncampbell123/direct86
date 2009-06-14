
#define MAX_DEVICES				256

typedef struct {
	int				plugged_in;
	char*			device_name;			// pointer to a string describing the device
	void			(*initdev)();			// callback when device initialized (emulation begins)
	void			(*closedev)();			// callback when device uninitialized (emulation ends)
	void			(*powerondev)();		// callback when computer "powered on"
	void			(*poweroffdev)();		// callback when computer "powered off"
	void			(*resetdev)();			// callback when computer "reset"
	void			(*buscycledev)();		// callback for every "bus cycle"
	int				(*is_powered)();		// callback for an "are you powered?" function
} EMUDEVICE;

extern EMUDEVICE					sys_devices[MAX_DEVICES];
extern BOOL							PowerOn;
extern BOOL							CPUpower;
extern int							CPUpower_max;
extern int							CPUpower_min;
extern int							CPUpower_lev;

void ComputerReset();
void ComputerInit();
void ComputerClose();
void ComputerPowerOn();
void ComputerPowerOff();
void ComputerCycle();
int ComputerAddDevice(EMUDEVICE *dev);
int ComputerRemoveDevice(int idx);
int ComputerResetDevice(int idx);
int ComputerPowerOffDevice(int idx);
int ComputerPowerOnDevice(int idx);
int ComputerIsDevicePowered(int idx);
void ComputerDevicesConfig();
