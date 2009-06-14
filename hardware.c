// hardware I/O simulator

#include "global.h"
#include "brkpts.h"
#include "addrbus.h"
#include "hardware.h"
#include "mother.h"

// callback functions for I/O ports
DWORD			((*io_port_call[65536])(int size,int direction,DWORD data));
int				hw_powered=0;

void HardOut(int port,BYTE data)
{
	if (!hw_powered) return;

	io_port_call[port&0xFFFF](1,IO_PORT_OUT,(DWORD)data);
	CheckTriggerIOBreakpoint((int)port,1,BPTR_WRITE);
}

BYTE HardIn(int port)
{
	if (!hw_powered) return 0xFF;

	CheckTriggerIOBreakpoint((int)port,1,BPTR_READ);
	return (BYTE)io_port_call[port&0xFFFF](1,IO_PORT_IN,0);
}

void HardOut16(int port,WORD data)
{
	if (!hw_powered) return;

	io_port_call[port&0xFFFF](2,IO_PORT_OUT,(DWORD)data);
	CheckTriggerIOBreakpoint((int)port,2,BPTR_WRITE);
}

WORD HardIn16(int port)
{
	if (!hw_powered) return 0xFF;

	CheckTriggerIOBreakpoint((int)port,2,BPTR_READ);
	return (WORD)io_port_call[port&0xFFFF](2,IO_PORT_IN,0);
}

void HardOut32(int port,DWORD data)
{
	if (!hw_powered) return;

	io_port_call[port&0xFFFF](4,IO_PORT_OUT,data);
	CheckTriggerIOBreakpoint((int)port,4,BPTR_WRITE);
}

DWORD HardIn32(int port)
{
	if (!hw_powered) return 0xFF;

	CheckTriggerIOBreakpoint((int)port,4,BPTR_READ);
	return io_port_call[port&0xFFFF](4,IO_PORT_IN,0);
}

// default behavior for unassigned I/O ports
DWORD default_io_call(int size,int direction,DWORD data)
{
	return 0xFFFFFFFF;
}

int assign_IO_port(int port,DWORD (*io_callback)(int,int,DWORD))
{
	if (!hw_powered) return 0;
	if (port < 0) return 0;
	if (port > 0xFFFF) return 0;

	io_port_call[port] = io_callback;

	return 1;
}

int unassign_IO_port(int port)
{
	if (!hw_powered) return 0;
	if (port < 0) return 0;
	if (port > 0xFFFF) return 0;

	io_port_call[port] = default_io_call;

	return 1;
}

void InitHardware()
{
	hw_powered=0;
}

void CloseHardware()
{
	hw_powered=0;
}

void ResetHardware()
{
	int x;

	for (x=0;x < 65536;x++) io_port_call[x] = default_io_call;
}

void CycleHardware()
{
}

void PowerOnHardware()
{
	hw_powered=1;
}

void PowerOffHardware()
{
	hw_powered=0;
}

int IsHardwarePowered()
{
	return hw_powered;
}
