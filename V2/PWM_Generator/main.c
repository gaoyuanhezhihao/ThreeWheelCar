#include <c8051f120.h>
#include "Init.h"
#include "Timer.h"

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define BAUDRATE     9600            // Baud rate of UART in bps

// SYSTEMCLOCK = System clock frequency in Hz
#define SYSTEMCLOCK       (22118400L * 9 / 4)

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
//----------------------PWM----------------------------------------------------
#define PWM1_HighLevelPercent 0.1
#define PWM3_HighLevelPercent 0.1
sbit PWM1 	= 	P3^2;                      
sbit PWM3   =   P3^3;
bit pwm1_flag=0;
bit pwm3_flag=0;
unsigned int PWM1_HighLevelCount=0;
unsigned int PWM1_LowLevelCount=0;
unsigned int PWM3_HighLevelCount=0;
unsigned int PWM3_LowLevelCount=0;


void main (void)
{
     
	SFRPAGE = CONFIG_PAGE;

	WDTCN = 0xDE;                       // Disable watchdog timer
	WDTCN = 0xAD;

	OSCILLATOR_Init ();                 // Initialize oscillator
	PORT_Init ();                       // Initialize crossbar and GPIO
	TIMER0_Init(SYSTEMCLOCK/2000);
	TIMER1_Init(SYSTEMCLOCK/2000);

	EA = 1;
	while (1)
	{  
		;
	}
}
