#define DIR_L_2 9
#define DIR_R_2 11
#define DIR_L_1 8
#define DIR_R_1 3
//use pin 11 on the mega for this example to work
#define PWM_1 9
#define PWM_2 10
#define TOTAL_BYTES 4
#define BEST_PWM 20000
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
  Serial1.begin(9600);
  Serial1.println();

  TCCR3A = TCCR3B = 0;
  TCCR3A = _BV(COM3A1) | _BV(WGM31); //non-inverting
  TCCR3B = _BV(CS30) | _BV(WGM32) | _BV(WGM33); //prescalar=1, fast pwm
  ICR3 = 800;
  OCR3A = 20;
  pinMode(5, OUTPUT);

  TCCR4A = TCCR4B = 0;
  TCCR4A = _BV(COM3A1) | _BV(WGM31); //non-inverting
  TCCR4B = _BV(CS30) | _BV(WGM32) | _BV(WGM33); //prescalar=1, fast pwm
  ICR4 = 800;
  OCR4A = 199;
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
    pinMode(PWM_1, INPUT);
    pinMode(PWM_2, INPUT);
    delay(500);//brake 500ms
    digitalWrite(DIR_L_2, LOW);
    digitalWrite(DIR_R_2, LOW);
    //pwmWriteHR(PWM_1, 65535);
    //pwmWriteHR(PWM_2, 65535);
    break;
  case 'f':
    state = 'f';
    Serial1.println("ok");
    Serial1.println("try to go forward");
    pinMode(PWM_1, OUTPUT);
    pinMode(PWM_2, OUTPUT);
    digitalWrite(DIR_L_2, HIGH);
    digitalWrite(DIR_R_2, HIGH);
    //pwmWriteHR(PWM_1, 32768);
    //pwmWriteHR(PWM_2, 32768);
    digitalWrite(DIR_L_1, 1);
    digitalWrite(DIR_R_1, 1);
    break;
  case 'l':
    state = 'l';
    Serial1.println("ok");
    Serial1.println("try to turn left");
    pinMode(PWM_1, OUTPUT);
    pinMode(PWM_2, OUTPUT);
    digitalWrite(DIR_L_2, HIGH);
    digitalWrite(DIR_R_2, HIGH);
    //pwmWriteHR(PWM_1, 32768);
    //pwmWriteHR(PWM_2, 32768);
    digitalWrite(DIR_L_1, 1);
    digitalWrite(DIR_R_1, 0);
    break;
  case 'r':
    state = 'r';
    Serial1.println("ok");
    Serial1.println("try to turn right");
    pinMode(PWM_1, OUTPUT);
    pinMode(PWM_2, OUTPUT);
    digitalWrite(DIR_L_2, HIGH);
    digitalWrite(DIR_R_2, HIGH);
    digitalWrite(DIR_L_1, 0);
    digitalWrite(DIR_R_1, 1);
    break;
  case 'b':
    state = 'b';
    Serial1.println("ok");
    Serial1.println("try to run back");
    pinMode(PWM_1, OUTPUT);
    pinMode(PWM_2, OUTPUT);
    digitalWrite(DIR_L_2, HIGH);
    digitalWrite(DIR_R_2, HIGH);
    digitalWrite(DIR_L_1, 0);
    digitalWrite(DIR_R_1, 0);
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