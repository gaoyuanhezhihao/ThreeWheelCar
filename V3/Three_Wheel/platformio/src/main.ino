#define DIR_L_1 8
#define DIR_L_2 9
#define DIR_R_1 10
#define DIR_R_2 11
#define TOTAL_BYTES 4
#define BEST_PWM 20000


char state = 's';
const char HEADER = 'H';
char stoped = 1;

void setup()
{
  Serial1.begin(9600);
  Serial1.println();
  pinMode(DIR_L_1, OUTPUT);
  pinMode(DIR_L_2, OUTPUT);
  pinMode(DIR_R_1, OUTPUT);
  pinMode(DIR_R_2, OUTPUT);

  TCCR3A = TCCR3B = 0;
  TCCR3A = _BV(COM3A1) | _BV(WGM31); //non-inverting
  TCCR3B = _BV(CS30) | _BV(WGM32) | _BV(WGM33); //prescalar=1, fast pwm
  ICR3 = 800;
  OCR3A = 255;
  pinMode(5, OUTPUT);

  TCCR4A = TCCR4B = 0;
  TCCR4A = _BV(COM3A1) | _BV(WGM31); //non-inverting
  TCCR4B = _BV(CS30) | _BV(WGM32) | _BV(WGM33); //prescalar=1, fast pwm
  ICR4 = 800;
  OCR4A = 255;
  pinMode(6, OUTPUT);
}

void change_pwm(char *rcv_ch, unsigned int *p_pwm,char channel)
{
  *p_pwm = rcv_ch[1] * 256;
  *p_pwm += (unsigned char)rcv_ch[2];
  Serial1.print("rcv:pwm=");
  Serial1.print(*p_pwm);
  if (channel == 'l')
  {
    OCR3A  = *p_pwm;
  }
  else
  {
    OCR4A = *p_pwm;
  }
  Serial1.println("ok\n");
  Serial1.print(channel);
  Serial1.println(" pwm changed\n");
}
void change_state(char order)
{
  change_wheel_direction(order);
}
void change_wheel_direction(char order)
{
  switch (order)
  {
  case 's':
    state = 's';
    Serial1.println("ok\n");
    Serial1.println("try to stop\n");
    stoped = 1;
    digitalWrite(DIR_L_1, 0);
    digitalWrite(DIR_L_2, 0);
    digitalWrite(DIR_R_1, 0);
    digitalWrite(DIR_R_2, 0);
    break;
  case 'f':
    state = 'f';
    Serial1.println("ok");
    Serial1.println("try to go forward");
    digitalWrite(DIR_L_1, 0);
    digitalWrite(DIR_L_2, 1);
    digitalWrite(DIR_R_1, 1);
    digitalWrite(DIR_R_2, 0);
    break;
  case 'l':
    state = 'l';
    Serial1.println("ok");
    Serial1.println("try to turn left");
    digitalWrite(DIR_L_1, 1);
    digitalWrite(DIR_L_2, 0);
    digitalWrite(DIR_R_1, 1);
    digitalWrite(DIR_R_2, 0);
    break;
  case 'r':
    state = 'r';
    Serial1.println("ok");
    Serial1.println("try to turn right");
    digitalWrite(DIR_L_1, 0);
    digitalWrite(DIR_L_2, 1);
    digitalWrite(DIR_R_1, 0);
    digitalWrite(DIR_R_2, 1);
    break;
  case 'b':
    state = 'b';
    Serial1.println("ok");
    Serial1.println("try to run back");
    digitalWrite(DIR_L_1, 1);
    digitalWrite(DIR_L_2, 0);
    digitalWrite(DIR_R_1, 0);
    digitalWrite(DIR_R_2, 1);
    break;
  default:
    break;
  }
}
void process_msg(char rcv_ch[3])
{
  static unsigned int pwm = 0;
  switch (rcv_ch[0])
  {
  case 'p':
    change_pwm(rcv_ch, &pwm,  'l');
    break;
  case 'q':
    change_pwm(rcv_ch, &pwm, 'r');
  default:
    change_state(rcv_ch[0]);
    break;
  }
}
void loop()
{
  //Serial1.println("hello");
  char rcv_ch[3] = { 0 };
  if (Serial1.available() >= TOTAL_BYTES)
  {
    char tag = Serial1.read();
    if (tag == HEADER)
    {
      rcv_ch[0] = Serial1.read();
      rcv_ch[1] = Serial1.read();
      rcv_ch[2] = Serial1.read();
      process_msg(rcv_ch);
    }
  }

}