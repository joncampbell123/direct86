// header file for hardware.cpp

#define IO_PORT_IN				1
#define IO_PORT_OUT				2

void HardOut(int port,BYTE data);
void HardOut16(int port,WORD data);
void HardOut32(int port,DWORD data);
BYTE HardIn(int port);
WORD HardIn16(int port);
DWORD HardIn32(int port);
int assign_IO_port(int port,DWORD (*io_callback)(int,int,DWORD));
int unassign_IO_port(int port);
void ResetHardware();
void CycleHardware();
void InitHardware();
void CloseHardware();
void PowerOnHardware();
void PowerOffHardware();
int IsHardwarePowered();
