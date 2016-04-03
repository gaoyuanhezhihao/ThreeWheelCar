#include "Control.h"
struct Pair_Angel_Control{
	int Angel;
	int Kp;
	int Ki;
	int Kd;
};



//-----------------------------------------------------------------------------
// 	Regulate
//-----------------------------------------------------------------------------
//	Regulate the Motors to make the car stay steady.
//-----------------------------------------------------------------------------
void Regulate(void)
{
	
	//Calculate the deviation 
	iDeviation = angle - ZeroPoint_angle - iStablePoint;
	uiAbsoluteDeviation = iDeviation&0x8000? -iDeviation:iDeviation;
	//Search the range the deviation belongs to
	
	//Calculate the PWM
	
	//Get the new rotate direction of the motor
	RotateDirection = ( iDeviation & 0x8000 ) >> 15 ;
	//has the direction been changed?
	if( cOldRotateDirection != RotateDirection)
	{
		//Brake
		IN11 = 1;
		IN12 = 1;
		IN31 = 1;
		IN32 = 1;
		
		cOldRotateDirection = RotateDirection;
	}
	else
	{
		//Change the rotate direction

		if(RotateDirection)
		{
			IN11 = 0;
			IN12 = 1;
			IN31 = 1;
			IN32 = 0;
		}
		else
		{
			IN11 = 1;
			IN12 = 0;
			IN31 = 0;
			IN32 = 1;
		} 
	}

	//Renew the Motor 
	PWMChange(1);
	PWMChange(2);
	return;
}