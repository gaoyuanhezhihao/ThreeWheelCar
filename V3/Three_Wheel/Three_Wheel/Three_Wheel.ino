void setup()
{
	TCCR3A = TCCR3B = 0;
	TCCR3A = _BV(COM3A1) | _BV(WGM31); //non-inverting
	TCCR3B = _BV(CS30) | _BV(WGM32) | _BV(WGM33); //prescalar=1, fast pwm
	ICR3 = 500;
	OCR3A = 20;
	pinMode(5, OUTPUT);

	TCCR4A = TCCR4B = 0;
	TCCR4A = _BV(COM3A1) | _BV(WGM31); //non-inverting
	TCCR4B = _BV(CS30) | _BV(WGM32) | _BV(WGM33); //prescalar=1, fast pwm
	ICR4 = 500;
	OCR4A = 199;
	pinMode(6, OUTPUT);
}

void loop() {
	//while (OCR3A<500) {
	//  OCR3A += 80;
	//  delay(500);
	//}
	//OCR3A = 6;
}