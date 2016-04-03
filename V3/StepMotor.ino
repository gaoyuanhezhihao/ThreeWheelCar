/*
By default Arduino's analogWrite (and consequently pwmWrite() since it mimics analogWrite()) uses 8 bit
pwm across all timers. 8 bit PWM allows for a total of 256 possible values. This library has some methods
for fine tuning resolution if a higher resolution is needed:

void pwmWriteHR(uint8_t pin, uint16_t duty)
Same a pwmWrite, except the range is 0 - 65535 (16 bit) instead
of 0 - 255 (8 bit)

float TimerX_GetResolution() (replace X with a timer number)
Gets the timer's resolution in base 2. The value returned in other words
represents the number of bits required to represent the timer's range. ie
the value 7 means it takes 7 bits to represent all possible pin duties at
that frequency, or 7-bit resolution. Note that a float is returned, not
an int.

float GetPinResolution(uint8_t pin)
Returns the same value as TimerX_GetResolution(), but takes a pin number
as a parameter so that the caller is timer agnostic.

There are several things to keep in mind when trying to optimize resolution:
-pwmWriteHR() is only useful for 16 bit timers, since 8 bit timers are inherently limited to 8 bit pwm
-The higher the frequency, the lower the resolution. pwmWriteHR() will automatically map input to the
actual timer resolution
-Resolution can be optimized in this way for 2 pins on the Uno (9 and 10), and 11 pins on the Mega (2,
3, 5, 6, 7, 8, 11, 12, 44, 45,  and 46)

Use the serial moniter to see output from this program
This example runs on mega and uno.
*/

#include <PWM.h>
#define ENA_1 4
#define ENA_2 5
#define DIR_1 2
#define DIR_2 3
//use pin 11 on the mega for this example to work
#define PWM_1 9
#define PWM_2 10
#define TOTAL_BYTES 4
#define BEST_PWM 4800
#define START_FIRST_PWM 3200
#define START_STEPS 20
#define START_PWM_STEP (( BEST_PWM - START_FIRST_PWM)/START_STEPS)
#define START_TIME 1000 //1000ms = 1s
#define STEP_TIME (START_TIME/START_STEPS)

char state = 's';
const char HEADER = 'H';
char stoped = 1;
void setup()
{
	InitTimersSafe(); //initialize all timers except for 0, to save time keeping functions
	Serial.begin(9600);
	Serial.println();

	pinMode(DIR_1, OUTPUT);
	pinMode(DIR_2, OUTPUT);
	pinMode(PWM_1, INPUT);
	pinMode(PWM_2, INPUT);
	pinMode(ENA_1, OUTPUT);
	pinMode(ENA_2, OUTPUT);
	digitalWrite(ENA_1, LOW);
	digitalWrite(ENA_2, LOW);
}

//void demonstrateFrequencysEffectOnResolution()
//{
//	Serial.println("As frequency increases, resolution will decrease...");
//	for (int i = 1; i < 10000; i += 10)
//	{
//		SetPinFrequency(PWM_1, i);  //setting the frequency
//
//		uint16_t frequency = Timer1_GetFrequency();
//		uint16_t decimalResolution = Timer1_GetTop() + 1;
//		uint16_t binaryResolution = GetPinResolution(PWM_1); //this number will be inaccurately low because the float is being truncated to a int
//
//		char strOut[75];
//		sprintf(strOut, "Frequency: %u Hz\r\n Number of Possible Duties: %u\r\n Resolution: %u bit\r\n", frequency, decimalResolution, binaryResolution);
//
//		Serial.println(strOut);
//	}
//
//	Serial.println("...Finished");
//}

void settingHighResolutionDuty(unsigned int pwm)
{
	SetPinFrequency(PWM_1, pwm); //setting the frequency to 10 Hz
	SetPinFrequency(PWM_2, pwm);
	//Serial.println("\r\npwmWrite() and pwmWriteHR() are identical except for the valid range of inputs.\r\nThe following loop calls both functions to produce the same result on the \r\nLED pin. The pin should to run 10Hz at 50% duty regardless of the function called.\r\n");

	//the PWM_1 should flicker (10Hz 50% duty) for 1 second before calling
	//the other function. This demonstrates the use of pwmWriteHR() and how its
	//use is nearly identical to pwmWrite()

		//setting the duty to 50% with 8 bit pwm. 128 is 1/2 of 256
		//pwmWrite(PWM_1, 128);
		//pwmWrite(PWM_2, 128);
		//Serial.println("8-Bit PWM");
		//delay(1000);

		//setting the duty to 50% with the highest possible resolution that 
		//can be applied to the timer (up to 16 bit). 1/2 of 65536 is 32768.
		pwmWriteHR(PWM_1, 32768);
		pwmWriteHR(PWM_2, 32768);
		//Serial.println("High Resolution PWM");
		//delay(1000);

}
void loose_start_car(char order, unsigned int last_pwm, unsigned int first_pwm, unsigned int step)
{
	int i = 0;
	settingHighResolutionDuty(first_pwm);
	change_wheel_direction(order);
	pinMode(PWM_1, OUTPUT);
	pinMode(PWM_2, OUTPUT);
	Serial.print(i*step + first_pwm);
	Serial.print(";");
	for (i = 0; i < START_STEPS; i++)
	{
		settingHighResolutionDuty(i*step + first_pwm);
		Serial.print(i*step + first_pwm);
		Serial.print(";");
		delay(STEP_TIME);
	}
	settingHighResolutionDuty(last_pwm);
}
void change_pwm(char *rcv_ch, unsigned int *p_pwm, unsigned int *p_first_pwm, unsigned int *p_pwm_step)
{
	*p_pwm = rcv_ch[1] * 256;
	*p_pwm += (unsigned char)rcv_ch[2];
	Serial.print("rcv:pwm=");
	Serial.print(*p_pwm);
	*p_first_pwm = (*p_pwm) * 2 /3 ;
	*p_pwm_step = (*p_pwm - *p_first_pwm) / START_STEPS;
	Serial.println("start_pwm:");
	Serial.print(*p_first_pwm);

	SetPinFrequency(PWM_1, *p_pwm);
	SetPinFrequency(PWM_2, *p_pwm);
	pwmWriteHR(PWM_1, 32768);
	pwmWriteHR(PWM_2, 32768);
	Serial.println("ok\n");
	Serial.println("pwm changed\n");
}
void change_state(char order, unsigned int pwm, unsigned int start_pwm, unsigned pwm_step)
{
	//char active_states[5] = {'f', 'b', 'r', 'l', '\0';
	//char *p_state = "fbrl";
	char *active_states = "fbrl";

	if (state == 's' && (order == 'f' || order == 'b' /*|| order == 'l' || order == 'r'*/))
	{
		stoped = 0;
		int i = 0;
		loose_start_car(order, pwm, start_pwm, pwm_step);
		return;
	}
	change_wheel_direction(order);
	//if (order == 's')
	//{
	//	change_wheel_direction(order);
	//}
	//else if (order != state)
	//{
	//	while (*active_states != '\0')
	//	{
	//		Serial.print(*active_states);
	//		if (*active_states == order)
	//		{
	//			// soft change
	//			change_wheel_direction('s');
	//			delay(100); //wait 100ms=0.1s
	//			change_wheel_direction(order);
	//			break;
	//		}
	//		active_states++;
	//	}
	//}
}
void change_wheel_direction(char order)
{
	switch (order)
	{
	case 's':
		state = 's';
		Serial.println("ok\n");
		Serial.println("try to stop\n");
		stoped = 1;
		pinMode(PWM_1, INPUT);
		pinMode(PWM_2, INPUT);
		delay(500);//brake 500ms
		digitalWrite(ENA_1, LOW);
		digitalWrite(ENA_2, LOW);
		//pwmWriteHR(PWM_1, 65535);
		//pwmWriteHR(PWM_2, 65535);
		break;
	case 'f':
		state = 'f';
		Serial.println("ok");
		Serial.println("try to go forward");
		pinMode(PWM_1, OUTPUT);
		pinMode(PWM_2, OUTPUT);
		digitalWrite(ENA_1, HIGH);
		digitalWrite(ENA_2, HIGH);
		//pwmWriteHR(PWM_1, 32768);
		//pwmWriteHR(PWM_2, 32768);
		digitalWrite(DIR_1, 1);
		digitalWrite(DIR_2, 1);
		break;
	case 'l':
		state = 'l';
		Serial.println("ok");
		Serial.println("try to turn left");
		pinMode(PWM_1, OUTPUT);
		pinMode(PWM_2, OUTPUT);
		digitalWrite(ENA_1, HIGH);
		digitalWrite(ENA_2, HIGH);
		//pwmWriteHR(PWM_1, 32768);
		//pwmWriteHR(PWM_2, 32768);
		digitalWrite(DIR_1, 1);
		digitalWrite(DIR_2, 0);
		break;
	case 'r':
		state = 'r';
		Serial.println("ok");
		Serial.println("try to turn right");
		pinMode(PWM_1, OUTPUT);
		pinMode(PWM_2, OUTPUT);
		digitalWrite(ENA_1, HIGH);
		digitalWrite(ENA_2, HIGH);
		digitalWrite(DIR_1, 0);
		digitalWrite(DIR_2, 1);
		break;
	case 'b':
		state = 'b';
		Serial.println("ok");
		Serial.println("try to run back");
		pinMode(PWM_1, OUTPUT);
		pinMode(PWM_2, OUTPUT);
		digitalWrite(ENA_1, HIGH);
		digitalWrite(ENA_2, HIGH);
		digitalWrite(DIR_1, 0);
		digitalWrite(DIR_2, 0);
		break;
	default:
		break;
	}
}
void process_msg(char rcv_ch[3])
{
	static unsigned int pwm = BEST_PWM;
	static unsigned int starting_first_pwm = START_FIRST_PWM;
	static unsigned int starting_pwm_step = START_PWM_STEP;
	switch (rcv_ch[0])
	{
	case 'p':
		change_pwm(rcv_ch, &pwm, & starting_first_pwm, & starting_pwm_step);
		break;
	default:
		change_state(rcv_ch[0], pwm, starting_first_pwm, starting_pwm_step);
		break;
	}
}
void loop()
{
	char rcv_ch[3] = {0};
	if (Serial.available() >= TOTAL_BYTES)
	{
		char tag = Serial.read();
		if (tag == HEADER)
		{
			rcv_ch[0] = Serial.read();
			rcv_ch[1] = Serial.read();
			rcv_ch[2] = Serial.read();
			process_msg(rcv_ch);
		}
	}

}
