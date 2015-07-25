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

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

#define UART_BUFFERSIZE 64
unsigned char UART_Buffer_Queue[UART_BUFFERSIZE];
unsigned char * UART_Buffer_QueueHead=UART_Buffer_Queue;
unsigned char * UART_Buffer_QueueBottom=UART_Buffer_Queue;
unsigned char TX_Ready =1;
static char Byte;
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
	TIMER0_Init(SYSTEMCLOCK/2000);
	TIMER1_Init(SYSTEMCLOCK/2000);

   UART0_Init ();                      // Initialize UART0
   WirelessModule_Init();
   EA = 1;

   SFRPAGE = UART0_PAGE;   

   while (1)
   {  
		Uart0_SendByte('h');
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

	XBR0     = 0x04;                    // Enable UART0
	XBR1     = 0x00;
	XBR2     = 0x40;                    // Enable crossbar and weak pull-up
                                       

	P0MDOUT |= 0x01;                    // Set TX pin to push-pull
	P1MDOUT |= 0x40;                    // Set P1.6(LED) to push-pull
	P3MDOUT = 0xFF;						// P3.x is open-drain
	SFRPAGE = SFRPAGE_SAVE;             // Restore SFR page
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

   if ( RI0 == 1 && (UART_Buffer_QueueBottom-UART_Buffer_Queue) < UART_BUFFERSIZE ) //if the buffer is not full
   {
      RI0 = 0;                           // Clear interrupt flag
      Byte = SBUF0;                      // Read a character from UART

      if ( UART_Buffer_QueueBottom < (UART_Buffer_Queue+UART_BUFFERSIZE) )
      {
			*UART_Buffer_QueueBottom = Byte; // Store in array
			UART_Buffer_QueueBottom++;
			if( UART_Buffer_QueueBottom >= (UART_Buffer_Queue+UART_BUFFERSIZE) )
				RerangeTheBufferQueue();
      }
	  else
		  RerangeTheBufferQueue();
   }

   if (TI0 == 1)                   // Check if transmit flag is set
   {
      TI0 = 0;                           // Clear interrupt flag

      TX_Ready = 1;                    // Indicate transmission complete
     
   }
}

//-----------------------------------------------------------------------------
// RerangeTheBufferQueue
//-----------------------------------------------------------------------------
//
// This routine is invoked to rerange the UART buffer queue when the bottom point has get to the bottom limit
//-----------------------------------------------------------------------------
void RerangeTheBufferQueue(void)
{
		int count = UART_Buffer_QueueBottom-UART_Buffer_QueueHead;
		int i=0;
		for(i=0;i<count;i++)
		{
				*(UART_Buffer_Queue+i)= UART_Buffer_QueueHead;
				UART_Buffer_QueueHead++;
				
		}
		UART_Buffer_QueueHead=UART_Buffer_Queue;
		UART_Buffer_QueueBottom=UART_Buffer_QueueHead+count;
}
//-----------------------------------------------------------------------------
// WirelessModule_Init
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void WirelessModule_Init(void)
{
	P3 = 0xFF; // Enter the SET mode of the wireless module
	strcat(UART_Buffer_Queue,"helloworld");
	UART_Buffer_QueueBottom += strlen("helloworld");
}
//-----------------------------------------------------------------------------
// Uart0_SendByte
//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void Uart0_SendByte(unsigned char value)
{
	char data SFRPAGE_SAVE = SFRPAGE;//save current SFR page
	SFRPAGE = UART0_PAGE;
	
	while(!TX_Ready);
	TX_Ready = 0;
	SBUF0 = value;
	
	
	SFRPAGE = SFRPAGE_SAVE;//Recover the SFR Page
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
	CKCON = 0X08;
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
	
	CKCON |= 0x10;
	TR1 = 0;
	TH1 = count;
	TL1 = count;
	ET1 = 1;
	TR1 =1;
	SFRPAGE = SFRPAGE_SAVE;
}
void Timer0_ISR(void) interrupt 1
{
	if(!pwm1_flag)
	{
		//Start of High level
			pwm1_flag = 1;	//Set flag
			PWM1 = 1;	//Set PWM o/p pin
			TH0 = (0xFFFF-PWM1_HighLevelCount)>>8;	//Load timer
			TL0 = (0xFFFF-PWM1_HighLevelCount);
			TF0 = 0;		//Clear interrupt flag
			return;		//Return
	}
	else
	{	//Start of Low level
		pwm1_flag = 0;	//Clear flag
		PWM1 = 0;	//Clear PWM o/p pin
		TH0 = (0xFFFF-PWM1_LowLevelCount)>>8;	//Load timer
		TL0 = (0xFFFF-PWM1_LowLevelCount);
		TF0 = 0;	//Clear Interrupt flag
		return;		//return
	}
}
void Timer1_ISR(void) interrupt 3
{
	if(!pwm3_flag)
	{
		//Start of High level
			pwm3_flag = 1;	//Set flag
			PWM3 = 1;	//Set PWM o/p pin
			TH1 = (0xFFFF-PWM3_HighLevelCount)>>8;	//Load timer
			TL1 = (0xFFFF-PWM3_HighLevelCount);
			TF1 = 0;		//Clear interrupt flag
			return;		//Return
	}
	else
	{	//Start of Low level
		pwm3_flag = 0;	//Clear flag
		PWM3 = 0;	//Clear PWM o/p pin
		TH1 = (0xFFFF-PWM3_LowLevelCount)>>8;	//Load timer
		TL1 = (0xFFFF-PWM3_LowLevelCount);
		TF1 = 0;	//Clear Interrupt flag
		return;		//return
	}
}
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------