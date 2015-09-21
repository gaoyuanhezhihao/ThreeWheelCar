#include "main.h"
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