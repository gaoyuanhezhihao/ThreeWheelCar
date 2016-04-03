//-----------------------------------------------------------------------------
// F12x_UART0_Interrupt.c
//-----------------------------------------------------------------------------
// ????????????? RJ ?? ???????
// ???????:	http://dadastudio.taobao.com/ 
// ????????	 C8051F12x-13x???.pdf ?? 21 ?:UART0
//
// Copyright 2006 Silicon Laboratories, Inc.
// http://www.silabs.com
//
// Program Description:
//
// This program demonstrates how to configure the C8051F120 to write to and read 
// from the UART interface. The program reads a word using the UART0 interrupts 
// and outputs that word to the screen, with all characters in uppercase
//
// How To Test:
//
// 1) Download code to a 'F12x device that is connected to a UART transceiver
// 2) Verify jumpers J6 and J9 are populated on the 'F12x TB.
// 3) Connect serial cable from the transceiver to a PC
// 4) On the PC, open HyperTerminal (or any other terminal program) and connect
//    to the COM port at <BAUDRATE> and 8-N-1
// 5) Download and execute code on an 'F12x target board.
// 6) Type up to 64 characters into the Terminal and press Enter.  The MCU 
//    will then print back the characters that were typed
//   
//
// Target:         C8051F12x
// Tool chain:     Keil C51 7.50 / Keil EVAL C51
// Command Line:   None
//
// Release 1.0
//    -Initial Revision (SM)
//    -11 JULY 2007
//
//	 P5.0-->PWMHigelevel bit0
//	 P5.1-->PWMHigelevel bit1
//	 P5.2-->PWMHigelevel bit2
//	 P5.3-->PWMHigelevel bit3
//   P5.4-->PWMHigelevel bit4
//	 P5.5-->PWMHigelevel bit5
//   P5.6-->PWMHigelevel bit6
//	 P5.7-->PWMHigelevel bit7
//	 P4.6-->PWMHigelevel bit8
//	 P4.7-->PWMHigelevel bit9
//	 P4.0-->cPWM1ChangeOrder
//   P4.1-->cPWM2ChangeOrder

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <c8051f120.h>                 // SFR declarations
#include <stdio.h>                     
#include <string.h>
#include <math.h>
#include "MyDebug.h"
//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F12x
//-----------------------------------------------------------------------------

sfr16 RCAP2    = 0xca;                 // Timer2 capture/reload
sfr16 TMR2     = 0xcc;                 // Timer2

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define BAUDRATE     9600            // Baud rate of UART in bps
#define UART1BAUDRATE 115200		 
#define Const_Control_Time	10			//ADC cycle = Const_Control_Time*Timer0 cycle = 10*10ms=100ms
sbit PWM1 	= 	P3^2;                      
sbit IN11		=	P6^3;
sbit IN12		= 	P6^4;
sbit TMR3Debug 	=   P3^5;
sbit SET		=	P3^1;
sbit Key1		= 	P7^2;
sbit PWM3		=	P6^7;
sbit IN31		=	P6^5;
sbit IN32		=	P6^6;
sbit DEBUGPORT  =   P3^0;
sbit PWM1CHANGEORDER = P4^0;
sbit PWM2CHANGEORDER = P4^1;
sbit PWMDEGREE_HighBit0=P4^6;
sbit PWMDEGREE_HighBit1=P4^7;
sbit D0 = P5^0;
sbit D1 = P5^1;
sbit D2 = P5^2;
sbit D3 = P5^3;
sbit D4 = P5^4;
sbit D5 = P5^5;
sbit D6 = P5^6;
sbit D7 = P5^7;
sbit START_BALANCE = P3^5;
sbit L_UP = P3^6;
sbit R_UP = P3^7;
// SYSTEMCLOCK = System clock frequency in Hz
#define SYSTEMCLOCK       (22118400L * 9 / 4)
#define TIMER0CLOCK			(SYSTEMCLOCK/48)
#define TIMER1CLOCK 		(SYSTEMCLOCK/48)
#define TIMER3CLOCK			(SYSTEMCLOCK/48)
#define MAP_ADC_ANGEL_SIZE  13
#define UART_BUFFERSIZE 64
#define UART1_BUFFERSIZE 22
#define PWMDEGREE_Low8Bits P5

//--------------------------------POSE & Control-------------------------------------------
#define MAX_PWM 800
#define ANGELRANGE_SIZE 7
#define ANGELSCALE_10 0xF8E3  //-10
#define ANGELSCALE_5 0xFC71  //-5
#define ANGELSCALE_3 0xFDDD  //-3
#define ANGELSCALE0  0x0000  //0
#define ANGELSCALE3  0x0222  //3
#define ANGELSCALE5  0x038E  //5
#define ANGELSCALE10  0x071C  //10

#define ADJUST_THRESHOLD 100
#define PWM_CHG_GAP 12
#define MAX_CIRCLE 5
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void Ext_Interrupt_Init (void);
void OSCILLATOR_Init (void);         
void PORT_Init (void);
void TIMER0_Init(void);
void Delay_ms(unsigned int count);
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

//-------------------------------UART0-----------------------------------------
unsigned int UART0_Receive_Buffer_Size = 0;
unsigned char UART_Receive_Buffer_Queue[UART_BUFFERSIZE]={0};
unsigned char * UART_Receive_Buffer_QueueHead=UART_Receive_Buffer_Queue;
unsigned char * UART_Receive_Buffer_QueueBottom=UART_Receive_Buffer_Queue;
unsigned char UART_Transmit_Buffer_Queue[UART_BUFFERSIZE];
unsigned char * UART_Transmit_Buffer_QueueHead=UART_Transmit_Buffer_Queue;
unsigned char * UART_Transmit_Buffer_QueueBottom=UART_Transmit_Buffer_Queue;
unsigned char TX_Ready =1;
static   char Byte;
		 char Rcv_New=0;
//--------------------------------UART1-----------------------------------------
unsigned char 	Counter_UART1 = 0;
unsigned char   Flag_NewFrame = 0;
unsigned char 	UART1_Receive_Buffer_Queue[UART1_BUFFERSIZE]={0};
unsigned char * UART1_Receive_Buffer_QueueHead   = UART1_Receive_Buffer_Queue;
unsigned char * UART1_Receive_Buffer_QueueBottom  = UART1_Receive_Buffer_Queue;


//--------------------------------POSE & Control-------------------------------------------
int a,angle,Temp,w;
int ZeroPoint_a,ZeroPoint_w,ZeroPoint_angle,ZeroPoint_Temp;
int iDeviation = 0;
unsigned int uiAbsoluteDeviation =  0;
unsigned int AbsoluteW_ui = 0;
int iStablePoint = 0;
bit RotateDirection = 0;
int count_L = 0;
int count_R = 0;
int iAngleRange[] = 0;
struct Pair_Angel_Control{
	int Angel;
	int Kp;
	int Ki;
	int Kd;
};
struct Pair_Angel_Control Map_Angel_PID[ANGELRANGE_SIZE] = {{ANGELSCALE_10,5,0,1},\
																{ANGELSCALE_5,4,0,1},\
																{ANGELSCALE_3,3,0,1},\
																{ANGELSCALE0,1,0,1},\
																{ANGELSCALE3,2,0,1},\
																{ANGELSCALE5,3,0,1},\
																{ANGELSCALE10,5,0,1}};
int iCurrentKey = 0;
unsigned int KeepAliveTime_i=0;
char car_state = 0;  //1 = it is go forwarding state now
//-------------------------------Motor--------------------------------
unsigned int Motor1_Time=0;
unsigned int Motor2_Time=0;

unsigned int PWM1_HighLevelCount=0;
unsigned int PWM1_LowLevelCount=0;
float fPWM1_HighLevelPercent = 0.5f;
bit pwm1_flag=0;

unsigned int PWM3_HighLevelCount=0;
unsigned int PWM3_LowLevelCount=0;
float fPWM3_HighLevelPercent = 0.9f;
bit pwm3_flag=0;

unsigned char TH1_HighLevelPrefetch = 0;
unsigned char TL1_HighLevelPrefetch = 0;
unsigned char TMRH3_HighLevelPrefetch = 0;
unsigned char TMRL3_HighLevelPrefetch = 0;

unsigned char TH1_LowLevelPrefetch = 0;
unsigned char TL1_LowLevelPrefetch = 0;
unsigned char TMRH3_LowLevelPrefetch = 0;
unsigned char TMRL3_LowLevelPrefetch = 0;

unsigned int uiPWM1Degree=0;
unsigned int uiPWM2Degree=0;
bit	PWM1ChangeOrder = 0;
bit	PWM2ChangeOrder = 0;

char cOldRotateDirection = 0;
//-------------------------------System--------------------------------
char data Global_SFRPAGE_SAVE;
//unsigned int i=0;
char cDebugTmp=0;
char Control_TimeIsUp = 0;
unsigned char Control_Time=0;


unsigned char TH0_Prefetch = 0;
unsigned char TL0_Prefetch = 0;
unsigned char PWM_debug1 = 0;
unsigned char PWM_debug2 = 0;
unsigned char PWM_debug3 = 0;
unsigned char PWM_debug4 = 0;
//unsigned int TMR3_1000_circles = 0;
//unsigned int TMR4_1000_circles = 0;

//-----------------------------------------------------------------------------
// main() Routine
//-----------------------------------------------------------------------------

void main (void)
{
	char flag0 = 0;
	char flag1 =0;
	unsigned long threshold_ul = PWM_CHG_GAP;
	unsigned int circle_ui = 0;
	unsigned int adjust_count = 1;
	//Initialization
	SFRPAGE = CONFIG_PAGE;
	WDTCN = 0xDE;                       // Disable watchdog timer
	WDTCN = 0xAD;
	OSCILLATOR_Init();  
	PORT_Init();                       // Initialize crossbar and GPIO
	Ext_Interrupt_Init();
	EA = 1;
	while(1)
	{
		L_UP = 0;
		R_UP = 0;
		if(!START_BALANCE)
		{
			threshold_ul = PWM_CHG_GAP;
			circle_ui = 0;
			count_R = 0;
			count_L = 0;
			adjust_count = 1;
		}
		else
		{
			if(count_L / adjust_count > PWM_CHG_GAP || count_R / adjust_count > PWM_CHG_GAP)
			{
				adjust_count ++;
				if(count_L > 30000 && count_R > 30000)
				{
					count_L -= 30000;
					count_R -= 30000;
				}
				if(count_L -count_R > 6)
				{
					//Left is too fast
					L_UP = 0;
					R_UP = 1;
					Delay_ms(5);
					L_UP = 0;
					R_UP = 0;
				}
				else if(count_R - count_L > 5)
				{
					// Right wheel is too fast
					L_UP = 1;
					R_UP = 0;
					Delay_ms(5);
					L_UP = 0;
					R_UP = 0;
				}
			}
		}
		
	}
}

//-----------------------------------------------------------------------------
// Initialization Subroutines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// OSCILLATOR_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function initializes the system clock to use the PLL as its clock
// source, where the PLL multiplies the external 22.1184MHz crystal by 9/4.
//
//-----------------------------------------------------------------------------
void OSCILLATOR_Init (void)
{
   int i=0;                              // Software timer

   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

   SFRPAGE = CONFIG_PAGE;              // Set SFR page

   OSCICN = 0x80;                      // Set internal oscillator to run
                                       // at its slowest frequency

   CLKSEL = 0x00;                      // Select the internal osc. as
                                       // the SYSTEMCLOCK source

   // Initialize external crystal oscillator to use 22.1184 MHz crystal

   OSCXCN = 0x67;                      // Enable external crystal osc.
    for(i=0; i < 256; )	// Wait at least 1ms
	{
		i++;
	}
   while (!(OSCXCN & 0x80));           // Wait for crystal osc to settle

   SFRPAGE = LEGACY_PAGE;
   FLSCL |=  0x30;                     // Initially set FLASH read timing for
                                       // 100MHz SYSTEMCLOCK (most conservative
                                       // setting)
   if (SYSTEMCLOCK <= 25000000L) {           
   // Set FLASH read timing for <=25MHz
      FLSCL &= ~0x30;
   } else if (SYSTEMCLOCK <= 50000000L) {    
   // Set FLASH read timing for <=50MHz
      FLSCL &= ~0x20;
   } else if (SYSTEMCLOCK <= 75000000L) {    
   // Set FLASH read timing for <=75MHz
      FLSCL &= ~0x10;
   } else {                            // set FLASH read timing for <=100MHz
      FLSCL &= ~0x00;
   }

   // Start PLL for 50MHz operation
   SFRPAGE = PLL0_PAGE;
   PLL0CN = 0x04;                      // Select EXTOSC as clk source
   PLL0CN |= 0x01;                     // Enable PLL power
   PLL0DIV = 0x04;                     // Divide by 4
   PLL0FLT &= ~0x0f;
   PLL0FLT |=  0x0f;                   // Set Loop Filt for (22/4)MHz input clock
   PLL0FLT &= ~0x30;                   // Set ICO for 30-60MHz
   PLL0FLT |=  0x10;

   PLL0MUL = 0x09;                     // Multiply by 9

   // wait at least 5us
   for (i = 0; i < 256; )
   {
	   i++;
   }

   PLL0CN |= 0x02;                     // Enable PLL

   while (PLL0CN & 0x10 == 0x00);      // Wait for PLL to lock

   SFRPAGE = CONFIG_PAGE;

   CLKSEL = 0x02;                      // Select PLL as SYSTEMCLOCK source

   SFRPAGE = SFRPAGE_SAVE;             // Restore SFRPAGE
}

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function configures the crossbar and GPIO ports.
//
// P0.0   digital   push-pull     UART TX
// P0.1   digital   open-drain    UART RX
//-----------------------------------------------------------------------------
void PORT_Init (void)
{
	char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

	SFRPAGE = CONFIG_PAGE;              // Set SFR page

//	XBR0     = 0x04;                    // Enable UART0
//	XBR0    |= 0x08;                     // Route CEX0 to P0.2
	
//	XBR1     = 0x20;					//Enable T2
	XBR2     = 0x40;                    // Enable crossbar and weak pull-up
//	XBR2	|= 0X08;					//Enable T4
	XBR1     = 0x04 + 0x10;				//Enable /INT0 and /INT1
//	XBR2	|= 0x04;					//Enable UART1
 //   P1MDIN   = 0xFF;                                   
	P0MDOUT |= 0x04;                    // Set CEX0 (P0.2) to push-pull
	P0MDOUT |= 0x01;                    // Set TX pin to push-pull
	P0MDOUT |= 0x04;					//Set UART1 TX pin to push-pull

	P3MDOUT = 0xF8;						// P3.3~7 is  push-pull
//	P6MDOUT = 0x60;						// P6.x is push-pull
	P6		&= ~0x80;
	P6MDOUT = 0xFF;
	P5MDOUT	= 0xFF;						//P6 is push-pull
	P4MDOUT = 0xC3;						//P4.7,4.6,4.0,4.1 is push-pull
//	P7MDOUT = 0x00;						//P7.x is  open-drain
	SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}

//-----------------------------------------------------------------------------
// TIMER0_Init
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
void TIMER0_Init(void)
{

	char data SFRPAGE_SAVE =SFRPAGE;
	SFRPAGE=TIMER01_PAGE;
	TCON &= ~0x30;//stop the timer0
	TMOD &= ~0x00;//Set the timer0 work in mode of hex
	TMOD |= 0x01;
	CKCON &= 0xf0;
	CKCON |= 0x02;
	CKCON &= ~0x08;//Timer 0 uses the clock defined by the prescale bits
	TR0 = 0;
	TH0= (0xFFFF-TIMER0CLOCK/20)>>8; //Timer0 cycle = 50ms
	TL0=  0xFFFF-TIMER0CLOCK/20;
	ET0 = 1;//Enable interrupt
	TR0 = 1;//Run Timer0
	SFRPAGE = SFRPAGE_SAVE;
}


//-----------------------------------------------------------------------------
// 	Delay_ms
//-----------------------------------------------------------------------------
//	input:count
//	output:void
//	make the Program wait for count ms
//-----------------------------------------------------------------------------
void Delay_ms(unsigned int count)
{
	unsigned char ii,jj;
	for(ii=0;ii<count;ii++)
	{
		for(jj=0;jj<250;jj++)
			;
	}
}


//-----------------------------------------------------------------------------
// /INT0 ISR
//-----------------------------------------------------------------------------
//
// Whenever a negative edge appears on P0.0, the LED is toggled.
// The interrupt pending flag is automatically cleared by vectoring to the ISR
//
//-----------------------------------------------------------------------------
void INT0_ISR (void) interrupt 0
{
	char data SFRPAGE_SAVE =SFRPAGE;
	SFRPAGE = CONFIG_PAGE;
	Delay_ms(1);
	count_L ++;

	SFRPAGE = SFRPAGE_SAVE;
}

//-----------------------------------------------------------------------------
// /INT1 ISR
//-----------------------------------------------------------------------------
//
// Whenever a negative edge appears on P0.1, the LED is toggled.
// The interrupt pending flag is automatically cleared by vectoring to the ISR
//
//-----------------------------------------------------------------------------
void INT1_ISR (void) interrupt 2
{
	char data SFRPAGE_SAVE =SFRPAGE;
	SFRPAGE = CONFIG_PAGE;
	Delay_ms(1);
	count_R ++;
	SFRPAGE = SFRPAGE_SAVE;
}
void Ext_Interrupt_Init (void)
{
   char SFRPAGE_SAVE = SFRPAGE;

   SFRPAGE = TIMER01_PAGE;

   TCON |= 0x05;                        // /INT 0 and /INT 1 are falling edge
                                       // triggered

   EX0 = 1;                            // Enable /INT0 interrupts
   EX1 = 1;                            // Enable /INT1 interrupts

   SFRPAGE = SFRPAGE_SAVE;
}
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------