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


//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include <c8051f120.h>                 // SFR declarations
#include <stdio.h>                     
#include <string.h>
//-----------------------------------------------------------------------------
// 16-bit SFR Definitions for 'F12x
//-----------------------------------------------------------------------------

sfr16 RCAP2    = 0xca;                 // Timer2 capture/reload
sfr16 TMR2     = 0xcc;                 // Timer2

//-----------------------------------------------------------------------------
// Global Constants
//-----------------------------------------------------------------------------

#define BAUDRATE     9600            // Baud rate of UART in bps

// SYSTEMCLOCK = System clock frequency in Hz
#define SYSTEMCLOCK       (22118400L * 9 / 4)

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------

void OSCILLATOR_Init (void);         
void PORT_Init (void);
void UART0_Init (void);
void RerangeTheBufferQueue(void);
void WirelessModule_Init(void);
void Uart0_SendByte(unsigned char value);
void TIMER0_Init(unsigned int count);
void TIMER1_Init(unsigned int count);
void Ext_Interrupt_Init (void);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

#define UART_BUFFERSIZE 64
unsigned char UART_Buffer_Queue[UART_BUFFERSIZE];
unsigned char * UART_Buffer_QueueHead=UART_Buffer_Queue;
unsigned char * UART_Buffer_QueueBottom=UART_Buffer_Queue;
unsigned char TX_Ready =1;
static char Byte;
char TxOrder = 0;
unsigned char TxByte = 0;
//----------------------PWM----------------------------------------------------
#define MOTOR_SPEED_DEGREE = 16;
//unsigned int Motor_speed_degree[16][
float PWM1_HighLevelPercent = 0.1;
float PWM3_HighLevelPercent = 0.1;
sbit PWM1 	= 	P4^4;                      
sbit PWM3   =   P3^4;
bit pwm1_flag=0;
bit pwm3_flag=0;
unsigned char INT01_Flag = 0;
unsigned int PWM1_HighLevelCount=0;
unsigned int PWM1_LowLevelCount=0;
unsigned int PWM3_HighLevelCount=0;
unsigned int PWM3_LowLevelCount=0;
unsigned char PWM_Degree_uc = 0;
//---------------------------Timer---------------------------------------------
unsigned int TH0_high_pre =  0;
unsigned int TL0_high_pre =  0;
unsigned int TH0_low_pre =  0;
unsigned int TL0_low_pre =  0;

unsigned int TH1_high_pre =  0;
unsigned int TL1_high_pre =  0;
unsigned int TH1_low_pre =  0;
unsigned int TL1_low_pre =  0;
int count = 0;
//-----------------------------------------------------------------------------
// main() Routine
//-----------------------------------------------------------------------------

void main (void)
{
	
	SFRPAGE = CONFIG_PAGE;

	WDTCN = 0xDE;                       // Disable watchdog timer
	WDTCN = 0xAD;

	OSCILLATOR_Init ();                 // Initialize oscillator
	PORT_Init ();                       // Initialize crossbar and GPIO
	TIMER0_Init(SYSTEMCLOCK/5000);
	TIMER1_Init(SYSTEMCLOCK/5000);
	Ext_Interrupt_Init();
//   UART0_Init ();                      // Initialize UART0
//   WirelessModule_Init();
	EA = 1;

	count = SYSTEMCLOCK/5000;


   while (1)
   {
	    ;
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
   int i;                              // Software timer

   char SFRPAGE_SAVE = SFRPAGE;        // Save Current SFR page

   SFRPAGE = CONFIG_PAGE;              // Set SFR page

   OSCICN = 0x80;                      // Set internal oscillator to run
                                       // at its slowest frequency

   CLKSEL = 0x00;                      // Select the internal osc. as
                                       // the SYSTEMCLOCK source

   // Initialize external crystal oscillator to use 22.1184 MHz crystal

   OSCXCN = 0x67;                      // Enable external crystal osc.
   for (i=0; i < 256; i++);            // Wait at least 1ms

   while (!(OSCXCN & 0x80));           // Wait for crystal osc to settle

   SFRPAGE = LEGACY_PAGE;
   FLSCL |=  0x30;                     // Initially set FLASH read timing for
                                       // 100MHz SYSTEMCLOCK (most conservative
                                       // setting)
   if (SYSTEMCLOCK <= 25000000) {           
   // Set FLASH read timing for <=25MHz
      FLSCL &= ~0x30;
   } else if (SYSTEMCLOCK <= 50000000) {    
   // Set FLASH read timing for <=50MHz
      FLSCL &= ~0x20;
   } else if (SYSTEMCLOCK <= 75000000) {    
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
   for (i = 0; i < 256; i++) ;

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

	XBR1     = 0x04 + 0x10;				//Enable /INT0 and /INT1
	XBR2     = 0x40;                    // Enable crossbar and weak pull-up
                                       

	P1MDOUT |= 0x40;                    // Set P1.6(LED) to push-pull
	P3MDOUT |= 0x10;					// P3.4 is push-pull
	P4MDOUT |= 0x10; 					// P4.4. is push-pull
	P5MDOUT = 0xFF; 					// P5.x is push-pull
	SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
}



void TIMER0_Init(unsigned int count)
{

	char data SFRPAGE_SAVE =SFRPAGE;
	PWM1_HighLevelCount = count * (1-PWM1_HighLevelPercent);
	PWM1_LowLevelCount = (float)count * PWM1_HighLevelPercent;
	SFRPAGE=TIMER01_PAGE;
	TCON &= ~0x30;//stop the timer0
	TMOD &= ~0x00;//Set the timer0 work in mode of hex
	TMOD |= 0x01;
	CKCON &= ~0x0F; //clear Timer0 part 
	CKCON |= 0x08;// use SysClk
	TR0 = 0;
	TH0 = count;
	TL0 = count;
	ET0 = 1;
	TR0 =1;
	SFRPAGE = SFRPAGE_SAVE;
}
void TIMER1_Init(unsigned int count)
{
	char data SFRPAGE_SAVE =SFRPAGE;
	PWM3_HighLevelCount = count * (1-PWM3_HighLevelPercent);
	PWM3_LowLevelCount = (float)count * PWM3_HighLevelPercent;
	SFRPAGE=TIMER01_PAGE;
	TCON &= ~0xC0;//stop timer1 ,clear TF1
	
	TMOD &= ~0x30;//Set the timer1 work in mode of hex
	TMOD |= 0x10;
	CKCON |= 0x10;//Use System clock
	TR1 = 0;
	TH1 = count;
	TL1 = count;
	ET1 = 1;
	TR1 =1;
	SFRPAGE = SFRPAGE_SAVE;
}
//-----------------------------------------------------------------------------
// Ext_Interrupt_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This function configures and enables /INT0 and /INT1 (External Interrupts)
// as negative edge-triggered.
//
//-----------------------------------------------------------------------------
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


void Timer0_ISR(void) interrupt 1
{
	char data SFRPAGE_SAVE =SFRPAGE;
	SFRPAGE=TIMER01_PAGE;
	
	if(!pwm1_flag)
	{
		//Start of High level
			pwm1_flag = 1;	//Set flag
			SFRPAGE = CONFIG_PAGE;
			PWM1 = 1;	//Set PWM o/p pin
		    SFRPAGE=TIMER01_PAGE;
//			TH0 = (0xFFFF-PWM1_HighLevelCount)>>8;	//Load timer
//			TL0 = (0xFFFF-PWM1_HighLevelCount);
			TH0 = TH0_high_pre;
			TL0 = TL0_high_pre;
			TF0 = 0;		//Clear interrupt flag
			return;		//Return
	}
	else
	{	//Start of Low level
		pwm1_flag = 0;	//Clear flag
		SFRPAGE = CONFIG_PAGE;
		PWM1 = 0;	//Clear PWM o/p pin
		SFRPAGE=TIMER01_PAGE;		
//		TH0 = (0xFFFF-PWM1_LowLevelCount)>>8;	//Load timer
//		TL0 = (0xFFFF-PWM1_LowLevelCount);
		TH0 = TH0_low_pre;
		TL0 = TL0_low_pre;
		TF0 = 0;	//Clear Interrupt flag
		return;		//return
	}
	SFRPAGE = SFRPAGE_SAVE;
}
void Timer1_ISR(void) interrupt 3
{
	char data SFRPAGE_SAVE =SFRPAGE;
	SFRPAGE=TIMER01_PAGE;
	if(!pwm3_flag)
	{
		//Start of High level
			pwm3_flag = 1;	//Set flag
			PWM3 = 1;	//Set PWM o/p pin
//			TH1 = (0xFFFF-PWM3_HighLevelCount)>>8;	//Load timer
//			TL1 = (0xFFFF-PWM3_HighLevelCount);
			TH1 = TH1_high_pre;
			TL1 = TL1_high_pre;
			TF1 = 0;		//Clear interrupt flag
			return;		//Return
	}
	else
	{	//Start of Low level
		pwm3_flag = 0;	//Clear flag
		PWM3 = 0;	//Set PWM o/p pin
//		TH1 = (0xFFFF-PWM3_LowLevelCount)>>8;	//Load timer
//		TL1 = (0xFFFF-PWM3_LowLevelCount);
		TH1 = TH1_low_pre;
		TL1 = TL1_low_pre;
		TF1 = 0;	//Clear Interrupt flag
		return;		//return
	}
	SFRPAGE = SFRPAGE_SAVE;
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
	PWM_Degree_uc = P5;
	SFRPAGE = SFRPAGE_SAVE;
	PWM1_HighLevelPercent = PWM_Degree_uc / 256.0;
	PWM1_HighLevelCount = count * (1-PWM1_HighLevelPercent);
	PWM1_LowLevelCount = (float)count * PWM1_HighLevelPercent;
	TH0_high_pre = (0xFFFF-PWM1_HighLevelCount)>>8;
	TL0_high_pre = (0xFFFF-PWM1_HighLevelCount);
	TH0_low_pre = (0xFFFF-PWM1_LowLevelCount)>>8;
	TL0_low_pre = (0xFFFF-PWM1_LowLevelCount);
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
	PWM_Degree_uc = P5;
	SFRPAGE = SFRPAGE_SAVE;
	PWM3_HighLevelPercent = PWM_Degree_uc / 256.0;
	PWM3_HighLevelCount = count * (1-PWM3_HighLevelPercent);
	PWM3_LowLevelCount = (float)count * PWM3_HighLevelPercent;
	TH1_high_pre = (0xFFFF-PWM3_HighLevelCount)>>8;
	TL1_high_pre = (0xFFFF-PWM3_HighLevelCount);
	TH1_low_pre = (0xFFFF-PWM3_LowLevelCount)>>8;	
	TL1_low_pre = (0xFFFF-PWM3_LowLevelCount);
}
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------