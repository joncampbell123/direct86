// timer emulation routines

#include "global.h"
#include "cpuexec.h"
#include "timer.h"
#include "addrbus.h"
#include "hardware.h"
#include "pic.h"

int timer_r_init=0;
int timer_sig=0;
int timer_power=0;

// Timer
void CALLBACK TimerProc(HWND hwnd,UINT uMsg,DWORD dwTime)
{
// signal only if the code is NOT in single-step mode
	if (code_freerun && !code_showallsteps) timer_sig=1;
}

// for one bus cycle
void TimerCycle()
{
	if (!timer_power) return;

	if (timer_sig) {
		timer_sig=0;
		HW_Signal_IRQ(0);
	}
}

// reset routine
void TimerReset()
{
	timer_sig=0;
}

void InitTimer()
{
	if (!timer_r_init) {
// set the timer up at standard 18.2 ticks/sec
		SetTimer(NULL,100,10000 / 182,(int (_stdcall*)())TimerProc);
		timer_r_init=1;
	}

	timer_power=0;
}

void CloseTimer()
{
	if (timer_r_init) {
		KillTimer(NULL,100);
		timer_r_init=0;
	}

	timer_power=0;
}

void TimerPowerDown()
{
	timer_power=0;
}

void TimerPowerUp()
{
	timer_power=1;
}

int TimerIsPowered()
{
	return timer_power;
}
