#ifndef MAIN
#define MAIN
	//-----------------------------------------------------------------------------
	// Global Constants
	//-----------------------------------------------------------------------------

	#define BAUDRATE     9600            // Baud rate of UART in bps

	// SYSTEMCLOCK = System clock frequency in Hz
	#define SYSTEMCLOCK       (22118400L * 9 / 4)
	//----------------------PWM----------------------------------------------------
	#define PWM1_HighLevelPercent 0.1
	#define PWM3_HighLevelPercent 0.1
	sbit PWM1 	= 	P3^2;                      
	sbit PWM3   =   P3^3;
	bit pwm1_flag=0;
	bit pwm3_flag=0;
	extern unsigned int PWM1_HighLevelCount=0;
	extern unsigned int PWM1_LowLevelCount=0;
	extern unsigned int PWM3_HighLevelCount=0;
	extern unsigned int PWM3_LowLevelCount=0;
#endif