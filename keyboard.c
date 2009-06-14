// PC/XT/AT+ keyboard controller emulation

#include "global.h"
#include "resource.h"
#include "kbdwin.h"
#include "keyboard.h"
#include "addrbus.h"
#include "hardware.h"
#include "mother.h"
#include "pic.h"

int KeyboardBufferPeek();

// input = to controller
// output = from controller
// these are status vars for keyboard controller emulation
BYTE KeyboardControllerOutput;
BOOL KeyboardControllerOutputReady;
BOOL KeyboardControllerInterrupt;
BYTE KeyboardControllerInput;
BOOL KeyboardControllerInputReady;
BOOL KeyboardControllerSelfTest=FALSE;
BOOL KeyboardControllerData=FALSE;
BYTE KeyboardControllerLastCmd=0;
int  KeyboardControllerInputWait=0; // simulation of wait involved with keyboard controller
int  keyboard_power=0;

// I/O port 60h (keyboard data)
DWORD keyctrl_60h(int size,int dir,DWORD data)
{
	DWORD r;

	if (dir == IO_PORT_IN) {
		KeyboardControllerOutputReady = FALSE;
		r = (DWORD)KeyboardControllerOutput;
	}

	if (dir == IO_PORT_OUT) {
		if (KeyboardControllerInputReady) {
			KeyboardControllerData = TRUE;
			KeyboardControllerInput = (BYTE)(data&0xFF);
			KeyboardControllerInputReady = FALSE;
			KeyboardControllerInputWait = 186;
		}
	}

	return r;
}

// I/O port 64h (keyboard command)
DWORD keyctrl_64h(int size,int dir,DWORD data)
{
	DWORD r;

	r=0;

	if (dir == IO_PORT_IN) {
		if (!KeyboardControllerData)
			r |= 0x08;
		if (KeyboardControllerSelfTest)
			r |= 0x04;
		if (!KeyboardControllerInputReady)
			r |= 0x02;
		if (KeyboardControllerOutputReady)
			r |= 0x01;
	}

	if (dir == IO_PORT_OUT) {
		data &= 0xFF;

		KeyboardControllerData = FALSE;
		KeyboardControllerInputReady = FALSE;
		KeyboardControllerInputWait = 2;

// Read Command Byte?
		if (data == 0x20) {
			KeyboardControllerOutput = KeyboardControllerInput;
			KeyboardControllerOutputReady = TRUE;
		}
// Self Test?
		if (data == 0xAA) {
			KeyboardControllerOutput = 0x55;
			KeyboardControllerOutputReady = TRUE;
		}
// Interface Test?
		if (data == 0xAB) {
			KeyboardControllerOutput = 0x00;
			KeyboardControllerOutputReady = TRUE;
		}
	}

	return r;
}

// called for every bus 'cycle'
void KeyboardControllerCycle()
{
	int d;

	if (!KeyboardControllerOutputReady) {
		d = KeyboardBufferRead();
		if (d >= 0) {
			KeyboardControllerOutput=(BYTE)d;
			KeyboardControllerOutputReady=TRUE;
			KeyboardControllerInterrupt=TRUE;
		}
	}
	else {
		d = KeyboardBufferPeek();
		if (d >= 0) KeyboardControllerInterrupt=TRUE;
	}

	if (KeyboardControllerInterrupt) {
		HW_Signal_IRQ(1);
		KeyboardControllerInterrupt=FALSE;
	}

	if (KeyboardControllerInputWait)
		KeyboardControllerInputWait--;
	else
		KeyboardControllerInputReady = TRUE;
}

// called when 'reset' is signalled
void KeyboardReset()
{
	if (!keyboard_power) return;

	KeyboardBufferReset();

	KeyboardControllerData=FALSE;
	KeyboardControllerSelfTest=FALSE;
	KeyboardControllerInputReady=TRUE;
	KeyboardControllerOutputReady=FALSE;
	KeyboardControllerLastCmd=0;
	KeyboardControllerInterrupt=FALSE;

// link to I/O ports 60h and 64h
	assign_IO_port(0x60,keyctrl_60h);
	assign_IO_port(0x64,keyctrl_64h);
}

void KeyboardInit()
{
	keyboard_power=0;
}

void KeyboardClose()
{
	unassign_IO_port(0x60);
	unassign_IO_port(0x64);
	keyboard_power=0;
	KeyboardBufferReset();
}

void KeyboardPowerOn()
{
	keyboard_power=1;
	KeyboardReset();
}

void KeyboardPowerOff()
{
	KeyboardClose();
}

int KeyboardIsPowered()
{
	return keyboard_power;
}
