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
#include <F120_FlashPrimitives.h>
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
sbit IN11		=	P3^3;
sbit IN12		= 	P3^4;
sbit SET		=	P3^1;
sbit Key1		= 	P7^2;
sbit PWM3		=	P6^7;
sbit IN31		=	P6^6;
sbit IN32		=	P6^5;
sbit DEBUGPORT  =   P3^0;
sbit PWM1CHANGEORDER = P4^0;
sbit PWM2CHANGEORDER = P4^1;
sbit PWMDEGREE_HighBit0=P4^6;
sbit PWMDEGREE_HighBit1=P4^7;
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
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void OSCILLATOR_Init (void);         
void PORT_Init (void);
void UART0_Init (void);
void RerangeTheBufferQueue(unsigned char IndexOfBuffer);
void WirelessModule_Init(void);
void Uart0_SendByte(unsigned char value);
void Uart0_TransmitString(unsigned char * pucString , int iStringSize );
void TIMER0_Init(void);
void TIMER1_Init(unsigned int count);
void Calibration(void);
void Delay_ms(unsigned int count);
void Regulate(void);
void FLASH_ByteWrite (FLADDR addr, char byte, bit SFLE);
unsigned char FLASH_ByteRead (FLADDR addr, bit SFLE);
void FLASH_PageErase (FLADDR addr, bit SFLE);
void SaveMapToFlash(void);
void RestoreMapFromFlash(void);
void TIMER3_Init(void);
void UART1_Init (void);
void Uart1_SendByte(unsigned char value);
void PWMChange(void);

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



//-----------------------------------------------------------------------------
// main() Routine
//-----------------------------------------------------------------------------

void main (void)
{

	//Initialization
	SFRPAGE = CONFIG_PAGE;
	WDTCN = 0xDE;                       // Disable watchdog timer
	WDTCN = 0xAD;
	OSCILLATOR_Init();  
	PORT_Init();                       // Initialize crossbar and GPIO
	UART0_Init();                      // Initialize UART0
//	UART1_Init();
	WirelessModule_Init();
//	TIMER0_Init();
	TIMER3_Init();
	EA = 1;

	//*************flash test**********************
//	FLASH_PageErase(0x8000,1);
//	for(i=0;i<MAP_ADC_ANGEL_SIZE;i++)
//	{
//		Map_ADC_Angel_array[i].ADC = i;
//		Map_ADC_Angel_array[i].Angel = -i;
//	}
//	SaveMapToFlash();
//	FLASH_PageErase(0x8000,1);
//	FLASH_ByteWrite(0x8000,'j',1);
//	FLASH_ByteWrite(0x8001,'a',1);
//	cReadFlash = FLASH_ByteRead(0x8000,1);
////*******************PWM test******************
//	uiPWM1Degree = 0;
//	while(1)
//	{
//		uiPWM1Degree +=50;
//		PWMChange(1);
//		PWMChange(2);
//	}
////-------------------PWM test------------------
	
	


	Uart0_TransmitString("Ready",strlen("Ready"));
	while (1)
	{
		if(UART_Receive_Buffer_QueueHead < UART_Receive_Buffer_QueueBottom)
		{
			//Uart0_SendByte(*UART_Receive_Buffer_Queue);
			//++UART_Receive_Buffer_Queue;
			if( 0x53 == *UART_Receive_Buffer_QueueHead )
			{
				++UART_Receive_Buffer_QueueHead;
				while( UART_Receive_Buffer_QueueBottom - UART_Receive_Buffer_QueueHead < 3 )
				{
					//wait the rest three char
					;
				}
				//check the sum 
				if( *UART_Receive_Buffer_QueueHead + *(UART_Receive_Buffer_QueueHead+1) != *(UART_Receive_Buffer_QueueHead+2) )
				{
					/*debug*/
					Uart0_SendByte('s');
					UART_Receive_Buffer_QueueHead+=3;
					/*debug end*/
					continue;
				}
				
				
				
				//next char (order) come
				switch(*UART_Receive_Buffer_QueueHead)
				{
					case 'g'://start the car
						uiPWM1Degree=uiPWM2Degree=*(UART_Receive_Buffer_QueueHead+1) * 100;
						PWMChange();
						IN11=1;
						IN12=0;
						IN31=1;
						IN32=0;
						break;
					case 'f'://forward
						uiPWM1Degree=uiPWM2Degree=*(UART_Receive_Buffer_QueueHead+1) * 100;
						PWMChange();						
						IN11=1;
						IN12=0;
						IN31=1;
						IN32=0;
						break;
					case 'l'://turn left
						uiPWM1Degree=uiPWM2Degree=*(UART_Receive_Buffer_QueueHead+1) * 100;
						PWMChange();						
						IN11=0;
						IN12=0;
						IN31=1;
						IN32=0;
						break;
					case 'r'://turn right
						uiPWM1Degree=uiPWM2Degree=*(UART_Receive_Buffer_QueueHead+1) * 100;
						PWMChange();						

						IN11=1;
						IN12=0;
						IN31=0;
						IN32=0;
						break;
					case 'b'://go back
						uiPWM1Degree=uiPWM2Degree=*(UART_Receive_Buffer_QueueHead+1) * 100;
						PWMChange();
						IN11=0;
						IN12=1;
						IN31=0;
						IN32=1;
						break;
					case 's'://stop
						uiPWM1Degree=uiPWM2Degree=0;
						PWMChange();
						IN11=0;
						IN12=0;
						IN31=0;
						IN32=0;
						break;
					default:
						/*debug*/
						Uart0_SendByte('n');
						/*debug end*/
						break;						
				}
			}
			else
			{
				++UART_Receive_Buffer_QueueHead;
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

	XBR0     = 0x04;                    // Enable UART0
//	XBR0    |= 0x08;                     // Route CEX0 to P0.2
	
	XBR1     = 0x20;					//Enable T2
	XBR2     = 0x40;                    // Enable crossbar and weak pull-up
	XBR2	|= 0X08;
	XBR2	|= 0x04;					//Enable UART1
 //   P1MDIN   = 0xFF;                                   
	P0MDOUT |= 0x04;                    // Set CEX0 (P0.2) to push-pull
	P0MDOUT |= 0x01;                    // Set TX pin to push-pull
	P0MDOUT |= 0x04;					//Set UART1 TX pin to push-pull

	P3MDOUT = 0x18;						// P3.3 P3.4 is  push-pull
//	P6MDOUT = 0x60;						// P6.x is push-pull
	P6		&= ~0x80;
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
	TH0= (0xFFFF-TIMER0CLOCK/100)>>8; //Timer0 cycle = 10ms
	TL0=  0xFFFF-TIMER0CLOCK/100;
	ET0 = 1;//Enable interrupt
	SFRPAGE = SFRPAGE_SAVE;
}

//-----------------------------------------------------------------------------
// TIMER1_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1)  int counts - calculated Timer overflow rate
//                    range is postive range of integer: 0 to 32767
//
// Configure Timer1 to auto-reload at interval specified by <counts> (no
// interrupt generated) using SYSCLK as its time base.
//
//-----------------------------------------------------------------------------
void TIMER1_Init(unsigned int count)
{
	char data SFRPAGE_SAVE =SFRPAGE;
	PWM1_HighLevelCount = count * fPWM1_HighLevelPercent;
	PWM1_LowLevelCount = (float)count * (1-fPWM1_HighLevelPercent);
	SFRPAGE=TIMER01_PAGE;
	TCON &= ~0xC0;//stop the timer1
	TMOD &= ~0x00;//Set the timer1 work in mode of hex
	TMOD |= 0x10;
	
	CKCON &=0x0f;//clear CKCON bit4~7
	CKCON |= 0x10;//Timer 1 use SYSTEMCLOCK
	
	TH1_HighLevelPrefetch = ( 0xffff - (unsigned int) (count * fPWM1_HighLevelPercent) )>>8;
	TL1_HighLevelPrefetch =   0xffff - (unsigned int) (count * fPWM1_HighLevelPercent) ;
	TH1_LowLevelPrefetch = (  0xffff - (unsigned int )( count * (1-fPWM1_HighLevelPercent) )  )>>8;
	TL1_LowLevelPrefetch =    0xffff - (unsigned int) (count * (1-fPWM1_HighLevelPercent));
	ET1 = 1;//Enable interrupt;
	TCON |= 0xC0;//start the timer1
	
	SFRPAGE = SFRPAGE_SAVE;
}

//-----------------------------------------------------------------------------
// TIMER3_Init
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void TIMER3_Init(void)
{
	char data SFRPAGE_SAVE =SFRPAGE;
//	PWM3_HighLevelCount = count * fPWM3_HighLevelPercent;
//	PWM3_LowLevelCount = (float)count * (1-fPWM3_HighLevelPercent);
	SFRPAGE=TMR3_PAGE;
	TMR3CN &= ~0x04;//stop the timer3
	TMR3CN &= ~0x01;//Auto-Reload Mode 
	TMR3CF &= ~0x18;//clear bit4 ,3.clock = SYSCLK/12
	
	RCAP3H = (0xFFFF-SYSTEMCLOCK/12/100)>>8; //Timer3 cycle = 10ms
	RCAP3L =  0xFFFF-TIMER0CLOCK/100;
//	TMRH3_HighLevelPrefetch = ( 0xffff - (unsigned int) (count * fPWM3_HighLevelPercent) )>>8;
//	TMRL3_HighLevelPrefetch =   0xffff - (unsigned int) (count * fPWM3_HighLevelPercent) ;
//	TMRH3_LowLevelPrefetch = (  0xffff - (unsigned int )( count * (1-fPWM3_HighLevelPercent) )  )>>8;
//	TMRL3_LowLevelPrefetch =  0xffff - (unsigned int) (count * (1-fPWM3_HighLevelPercent));
	
	EIE2 |= 0x01;//Enable TIMER3 interrupt;
	TMR3CN |= 0x04;//start the timer3
	SFRPAGE = SFRPAGE_SAVE;
}
void Timer3_ISR(void) interrupt 14
{
	char data SFRPAGE_SAVE =SFRPAGE;//Save current SFP page
	SFRPAGE = TMR3_PAGE;
	/***************Debug Begin***********************/
		DEBUGPORT ^= 1;
	//---------------Debug End ---------------------------
//	if( Motor1_Time == 0 )
//	{
//		IN11=0;
//		IN12=0;
//	}
//	else
//	{
//		Motor1_Time--;
//	}
//	
//	if( Motor2_Time == 0 )
//	{
//		IN31=0;
//		IN32=0;
//	}
//	else
//	{
//		Motor2_Time--;
//	}
	
	if( Control_Time == 0 )
	{
		Control_Time = Const_Control_Time;
		Control_TimeIsUp=1;
//		/***************Debug Begin***********************/
//		IN31 ^= 1;
//		//---------------Debug End ---------------------------
	}
	else
	{
		Control_Time--;
	}
	TMR3CN &= ~0x80;//clear interrupt flag
	SFRPAGE=Global_SFRPAGE_SAVE;
	SFRPAGE = SFRPAGE_SAVE;
}

//-----------------------------------------------------------------------------
// UART0_Init   Variable baud rate, Timer 2, 8-N-1
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configure UART0 for operation at <baudrate> 8-N-1 using Timer2 as
// baud rate source.
//
//-----------------------------------------------------------------------------
void UART0_Init (void)
{
   char SFRPAGE_SAVE;

   SFRPAGE_SAVE = SFRPAGE;             // Preserve SFRPAGE

   SFRPAGE = TMR2_PAGE;

   TMR2CN = 0x00;                      // Timer in 16-bit auto-reload up timer
                                       // mode
   TMR2CF = 0x08;                      // SYSCLK is time base; no output;
                                       // up count only
   RCAP2 = - ((long) SYSTEMCLOCK/BAUDRATE/16);
   TMR2 = RCAP2;
   TR2= 1;                             // Start Timer2

   SFRPAGE = UART0_PAGE;

   SCON0 = 0x50;                       // 8-bit variable baud rate;
                                       // 9th bit ignored; RX enabled
                                       // clear all flags
   SSTA0 = 0x15;                       // Clear all flags; enable baud rate
                                       // doubler (not relevant for these
                                       // timers);
                                       // Use Timer2 as RX and TX baud rate
                                       // source;
   ES0 = 1;  
   IP |= 0x10;

   SFRPAGE = SFRPAGE_SAVE;             // Restore SFRPAGE
}


//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Timer0_ISR
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
void Timer0_ISR(void) interrupt 1
{	
	Global_SFRPAGE_SAVE=SFRPAGE;//Save current SFP page
	SFRPAGE=CONFIG_PAGE;
	
	if( Motor1_Time == 0 )
	{
		IN11=0;
		IN12=0;
	}
	else
	{
		Motor1_Time--;
	}
	
	if( Motor2_Time == 0 )
	{
		IN31=0;
		IN32=0;
	}
	else
	{
		Motor2_Time--;
	}
	
	if( Control_Time == 0 )
	{
		Control_Time = Const_Control_Time;
		Control_TimeIsUp=1;
		/***************Debug Begin***********************/
//		IN31 ^= 1;
		//---------------Debug End ---------------------------
	}
	else
	{
		Control_Time--;
	}
	TF0=0;
	SFRPAGE=Global_SFRPAGE_SAVE;
}
//-----------------------------------------------------------------------------
// Timer1_ISR
//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
//void Timer1_ISR(void) interrupt 3
//{
//	char data SFRPAGE_SAVE =SFRPAGE;//Save current SFP page
//	if(!pwm1_flag)
//	{
//			
//		//Start of High level
//			pwm1_flag = 1;	//Set flag
//			SFRPAGE=CONFIG_PAGE;
//			PWM1 = 1;	//Set PWM o/p pin
//			
//			TH1 = TH1_HighLevelPrefetch;	//Load timer
//			TL1 = TL1_HighLevelPrefetch;
//			TF0 = 0;		//Clear interrupt flag
//			SFRPAGE=SFRPAGE_SAVE;
//			return;		//Return
//	}
//	else
//	{	//Start of Low level
//		pwm1_flag = 0;	//Clear flag
//		SFRPAGE=CONFIG_PAGE;
//		PWM1 = 0;	//Clear PWM o/p pin
//		
//		TH1 = TH1_LowLevelPrefetch;	//Load timer
//		TL1 = TL1_LowLevelPrefetch;
//		TF0 = 0;	//Clear Interrupt flag
//		
//		SFRPAGE=SFRPAGE_SAVE;
//		return;		//return
//	}
//}

//void Timer3_ISR(void) interrupt 14
//{
//	char data SFRPAGE_SAVE =SFRPAGE;//Save current SFP page
//	if(!pwm3_flag)
//	{
//			
//		//Start of High level
//			pwm3_flag = 1;	//Set flag
//			SFRPAGE = CONFIG_PAGE;
//			PWM3 = 1;	//Set PWM o/p pin
//		
//			SFRPAGE = TMR3_PAGE;
//			TMR3H = TMRH3_HighLevelPrefetch;	//Load timer
//			TMR3L = TMRL3_HighLevelPrefetch;
//						
//			TF3 = 0;		//Clear interrupt flag
//		
//			SFRPAGE = SFRPAGE_SAVE;
//			return;
//	}
//	else
//	{	//Start of Low level
//		pwm3_flag = 0;	//Clear flag
//		
//		SFRPAGE = CONFIG_PAGE;
//		PWM3 = 0;	//Set PWM o/p pin
//		
//		SFRPAGE = TMR3_PAGE;
//		TMR3H = TMRH3_LowLevelPrefetch;	//Load timer
//		TMR3L = TMRL3_LowLevelPrefetch;
//		
//		
//		TF3 = 0;	//Clear Interrupt flag
//		
//		SFRPAGE = SFRPAGE_SAVE;
//		return;	
//	}
//}
//-----------------------------------------------------------------------------
// UART0_Interrupt
//-----------------------------------------------------------------------------
//
// This routine is invoked whenever a character is entered or displayed on the
// Hyperterminal.
//
//-----------------------------------------------------------------------------

void UART0_Interrupt (void) interrupt 4
{
   SFRPAGE = UART0_PAGE;

   if ( RI0 == 1 && (UART_Receive_Buffer_QueueBottom-UART_Receive_Buffer_QueueHead) < UART_BUFFERSIZE ) //if the buffer is not full
   {
      
      Byte = SBUF0;                      // Read a character from UART

      if ( UART_Receive_Buffer_QueueBottom < (UART_Receive_Buffer_Queue+UART_BUFFERSIZE) )
      {
			*UART_Receive_Buffer_QueueBottom = Byte; // Store in array
			UART_Receive_Buffer_QueueBottom++;
			if( UART_Receive_Buffer_QueueBottom >= (UART_Receive_Buffer_Queue+UART_BUFFERSIZE) )
				RerangeTheBufferQueue('r');
			Rcv_New=1;//Notice new message come
			UART0_Receive_Buffer_Size++;
      }
	  else
		  RerangeTheBufferQueue('r');
	  RI0 = 0;                           // Clear interrupt flag
   }

   if (TI0 == 1)                   // Check if transmit flag is set
   {
	   TX_Ready = 1;
	   TI0 = 0;
//      if ( UART_Transmit_Buffer_QueueHead < UART_Transmit_Buffer_QueueBottom )         // If buffer not empty
//      {
////         Byte = UART_Buffer[UART_Output_First];
////         if ((Byte >= 0x61) && (Byte <= 0x7A)) { // If upper case letter
////            Byte -= 32; }
////					int count = UART_Transmit_Buffer_QueueBottom - UART_Transmit_Buffer_QueueHead;
////					for(;count>0;count--)
////					{
////						Uart0_SendByte(*UART_Transmit_Buffer_QueueHead);  // Transmit to Hyperterminal
////					    UART_Transmit_Buffer_QueueHead++;
////					}
////		  while( UART_Transmit_Buffer_QueueHead < UART_Transmit_Buffer_QueueBottom )
////		  {
//			  TI0 = 0;
//			  SBUF0 = *UART_Transmit_Buffer_QueueHead;
//			  while( !TI0 );
//			  ++UART_Transmit_Buffer_QueueHead;
////		  }
//      }
//      else
//	  {
//		  TX_Ready = 1;                    // Indicate transmission complete
//		  TI0 = 0;                           // Clear interrupt flag
//	  }
   }
}
//-----------------------------------------------------------------------------
// UART1_Interrupt
//-----------------------------------------------------------------------------
//
// This routine is invoked whenever a character is entered or displayed on the
// Hyperterminal.
//
//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// RerangeTheBufferQueue
//-----------------------------------------------------------------------------
//
// This routine is invoked to rerange the UART buffer queue when the bottom point has get to the bottom limit
//-----------------------------------------------------------------------------
void RerangeTheBufferQueue( char IndexOfBuffer)
{
	if( IndexOfBuffer == 'r')//rerange the receive buffer
	{
		int count = UART_Receive_Buffer_QueueBottom-UART_Receive_Buffer_QueueHead;
		int i=0;
		for(i=0;i<count;i++)
		{
				*(UART_Receive_Buffer_Queue+i)= UART_Receive_Buffer_QueueHead;
				UART_Receive_Buffer_QueueHead++;
				
		}
		UART_Receive_Buffer_QueueHead=UART_Receive_Buffer_Queue;
		UART_Receive_Buffer_QueueBottom=UART_Receive_Buffer_QueueHead+count;
	}
	
	if( IndexOfBuffer == 't')//rerange the transmit buffer
	{
		int count = UART_Transmit_Buffer_QueueBottom-UART_Transmit_Buffer_QueueHead;
		int i=0;  
		for(i=0;i<count;i++)
		{
				*(UART_Transmit_Buffer_Queue+i)= UART_Transmit_Buffer_QueueHead;
				UART_Transmit_Buffer_QueueHead++;
				
		}
		UART_Transmit_Buffer_QueueHead=UART_Transmit_Buffer_Queue;
		UART_Transmit_Buffer_QueueBottom=UART_Transmit_Buffer_QueueHead+count;
	}	
}
//-----------------------------------------------------------------------------
// WirelessModule_Init
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WirelessModule_Init(void)
{
	SET=1; // Enter the SET mode of the wireless module
	memset( UART_Transmit_Buffer_Queue,0,sizeof(UART_Transmit_Buffer_Queue) );
}
//-----------------------------------------------------------------------------
// Uart0_SendByte
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Uart0_SendByte(unsigned char value)
{
	char data SFRPAGE_SAVE = SFRPAGE;//save current SFR page
	int i = 0;
	SFRPAGE = UART0_PAGE;
//	//Check if the uart transmit is ready
//	while(1)
//	{
//		if( TI0 == 0 && TX_Ready == 1)
//		{
//			break;
//		}
//	}
//	
//	//Check if there are enough space
//	if( UART_Transmit_Buffer_QueueBottom - UART_Transmit_Buffer_Queue >= UART_BUFFERSIZE )
//	{
//		RerangeTheBufferQueue('t');
//	}
//	//Write the Byte to the Transmit queue
//	*UART_Transmit_Buffer_QueueBottom = value;
//	++UART_Transmit_Buffer_QueueBottom;
//	//Set the TI0 
//	TI0 = 1;
    while( TX_Ready == 0 );
	
	TI0 = 0;
	TX_Ready = 0;
	SBUF0 = value;
	while(!TX_Ready)
	{
		;
	}
	SFRPAGE = SFRPAGE_SAVE;//Recover the SFR Page
}
//-----------------------------------------------------------------------------
// Calibration
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Calibration(void)
{
	char a_flag = 0,w_flag = 0,angle_flag = 0;
	UART0_Receive_Buffer_Size = UART_Receive_Buffer_QueueBottom - UART_Receive_Buffer_QueueHead;
//	strcat(UART_Transmit_Buffer_QueueHead,"ReadyToAdjust");
//	UART_Transmit_Buffer_QueueBottom += strlen("ReadyToAdjust");
	Uart0_SendByte('R');
	
//	Global_SFRPAGE_SAVE = SFRPAGE;
//	SFRPAGE = ADC0_PAGE;
//	TX_Ready = 0;                  // Set the flag to zero
//    TI0 = 1;                       // Set transmit flag to 1
//	SFRPAGE = Global_SFRPAGE_SAVE;
	
	while(1)
	{
		while( Rcv_New == 0 );//Wait for new message
		Rcv_New = 0;
		
		if( *UART_Receive_Buffer_QueueHead++ != 'Z' )
		{
			continue;
		}
		else
		{
			while(1)
			{
				if(UART1_Receive_Buffer_Queue[0]==0x55)      
				{  
					switch(UART1_Receive_Buffer_Queue [1])
					{
						case 0x51:
						a = (int)(UART1_Receive_Buffer_Queue [3]<<8| UART1_Receive_Buffer_Queue [2]);
						Temp = (int)(UART1_Receive_Buffer_Queue [9]<<8| UART1_Receive_Buffer_Queue [8]);
						a_flag = 1;
						break;
						case 0x52:
						w = (int)(UART1_Receive_Buffer_Queue [3]<<8| UART1_Receive_Buffer_Queue [2]);
						w_flag = 1;
						break;
						case 0x53:
						angle = (int)((unsigned int)UART1_Receive_Buffer_Queue [3]<<8| UART1_Receive_Buffer_Queue [2]);
						angle_flag = 1;
						break;
					} 
				}
				
				if( a_flag + w_flag + angle_flag == 3 )
				{
					break;
				}
			}
			
			//Save the current state to ZeroPoint
			ZeroPoint_a = a;
			ZeroPoint_w = w;
			ZeroPoint_angle = angle;
			ZeroPoint_Temp = Temp;
			//
			SaveMapToFlash();
			return;
		}

	}
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





void Uart0_TransmitString(unsigned char * pucString , int iStringSize )
{
	unsigned char *pucHeadofString = pucString;
	if( strlen(pucString) != iStringSize )
	{
		ERROR("Uart0_TransmitString():string lenth not match");
	}
	

	while( *pucHeadofString != 0 )
	{
		Uart0_SendByte(*pucHeadofString);
		++pucHeadofString;
	}
	
	
}

void PWMChange(void)
{
	int i= 0;
	/*************************DEBUG BEGIN*************************/
	if( uiPWM1Degree > 800 )
	{
		ERROR("void PWMChange(char PWMChannel):PWMDegree is too large");
	}
	//------------------------DEBUG END-------------------------------
	PWMDEGREE_Low8Bits  = (unsigned char)uiPWM1Degree;
	PWMDEGREE_HighBit0 = (uiPWM1Degree & 0x100)?1:0;
	cDebugTmp = (uiPWM1Degree & 0x100)?1:0;
	PWMDEGREE_HighBit1 = (uiPWM1Degree & 0x200)?1:0;
	cDebugTmp = (uiPWM1Degree & 0x200)?1:0;
	
	PWM1CHANGEORDER =  0;
	while( PWM1CHANGEORDER != 0 );//wait
	PWM1CHANGEORDER =  1;
	PWM2CHANGEORDER =  0;
	while( PWM2CHANGEORDER != 0 );//wait
	PWM2CHANGEORDER =  1;
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------