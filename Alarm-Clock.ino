#include "Alarm-Clock.h";

/*
  Send out correct binary number using the data, clock,
  and latch.
*/
void displayDigitSerial(int num) {
  digitalWrite(LATCHPIN, LOW);
  shiftOut(DATAPIN, CLOCKPIN, LSBFIRST, numbers[num]);
  digitalWrite(LATCHPIN, HIGH);
}

/*
  Turn off the previous digit and set up the next digit.
*/
void displayDigit(int digit, int num) {
  digitalWrite(BD1, LOW);
  digitalWrite(BD2, LOW);
  displayDigitSerial(num);
  /*
   0 0 D1
   0 1 D2
   1 0 D3
   1 1 D4
  */
  digitalWrite(BD1, digit % 2); 
    // 2 - 1 = 1 = 0b01     0b01 % 2 = 1 (HIGH)
    // 4 - 1 = 3 = 0b11     0b11 % 2 = 1 (HIGH)
  digitalWrite(BD2, (digit / 2) % 2);
    // 3 - 1 = 2 = 0b10     0b10 / 2 = 0b1 % 2 = 1 (HIGH)
    // 4 - 1 = 2 = 0b11     0b11 / 2 = 0b1 % 2 = 1 (HIGH)
  
}

/*
  Display the provided hour and minute in military time.
  This is intended to work with a single shift register.
  Needs to be called every cycle.
*/
void displayFullTime(int hour, int minute) {
  values[0] = hour / 10;
  values[1] = hour % 10;
  values[2] = minute / 10;
  values[3] = minute % 10;

  for(int i = 0; i < 4; i++) {
    displayDigit(i, values[i]);
    delay(5);
  }
}

/*
  Check to see if alarm is to be turned off by reading mode pin
*/
void checkAlarm() {

  // Turn on alarm and force mode to time to display the time, alarm button now turns off alarm
  // if(clock.isAlarm1(false)) {

  if(dt.hour == at.hour && dt.minute == at.minute && dt.second == 0) {
    analogWrite(ALARMPIN, 255);
    alarmActive = true;
    mode = ALARMING;
  }
  // else if(clock.isAlarm1(false) && digitalRead(MODEPIN) == HIGH) {
  else if(alarmActive && digitalRead(MODEPIN) == HIGH) {
    alarmActive = false;
    // clock.clearAlarm1();
    // clock.armAlarm1();
    analogWrite(ALARMPIN, 0);
    mode = TIME;
  }
}

/*
  Set LEDs to correct form when a new mode occurs or when alarm signals
*/
void setLED() {
  Serial.println(mode);
  switch(mode) {
    case TIME:
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      break;
    case SET:
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);
      break;
    case SETALARM:
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, HIGH);
      break;
    case ALARMING:
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, HIGH);
      break;
  }
}

/*
  Check to see if mode button has been pressed,
  if so update accordingly and perform mode actions.
*/
void checkMode() {
  checkAlarm();
  // if(clock.isAlarm1(false)) {
  if(alarmActive) {
    return;
  }
  
  if(!activeMode && digitalRead(MODEPIN) == LOW) {
    // Skips last 2 modes since they are not used in this context
    mode = (Modes)(((int)mode + 1) % ((int)NUM_MODES - 1));
    activeMode = true;
  }
  else if(activeMode && digitalRead(MODEPIN) == HIGH) {
    activeMode = false;
  }
}

/*
  Set the actual clock to the requested time
*/
void setClock(int hour, int minute) {
  clock.setDateTime(dt.year, dt.month, dt.day, hour, minute, 0);
}

/*
  Set the alarm time to the requested time
*/
void setAlarm(int hour, int minute) {
  clock.setAlarm1(0, hour, minute, 0, DS3231_MATCH_H_M_S, false);
}

void getTime(int time[]) {
  time[0] = 0;
  time[1] = 0;

  switch(mode) {
    case SETALARM:
      currentCallback = setAlarm;
      setTime(at.hour, at.minute);
      time[0] = at.hour;
      time[1] = at.minute;
      break;

    case SET:
      currentCallback = setClock;
      setTime(dt.hour, dt.minute);
    case TIME:
    case ALARMING:
    default:
      time[0] = dt.hour;
      time[1] = dt.minute;
      break;
  }
}

/*
  Perform correct callback function to set the time for either 
  the RTCAlarmTime or the RTCDateTime  
*/
void setTime(int hour, int minute) {
  if(!activeHour && digitalRead(HOURPIN) == LOW) {
    currentCallback((hour + 1) % 24, minute);
    activeHour = true;
  }
  else if(activeHour && digitalRead(HOURPIN) == HIGH) {
    activeHour = false;
  }

  if(!activeMinute && digitalRead(MINPIN) == LOW) {
    currentCallback(hour, (minute + 1) % 60);
    activeMinute = true;
  }
  else if(activeMinute && digitalRead(MINPIN) == HIGH) {
    activeMinute = false;
  }
}


/*
  Set up initial settings for pin modes and voltages,
  clock communcation, and variables.
*/
void setup() {
  Serial.begin(9600);
  clock.begin();

  // clock.setDateTime(__DATE__, __TIME__);

  pinMode(BD1, OUTPUT);
  pinMode(BD2, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(DATAPIN, OUTPUT);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(MINPIN, INPUT_PULLUP);
  pinMode(HOURPIN, INPUT_PULLUP);
  pinMode(MODEPIN, INPUT_PULLUP);
  pinMode(ALARMPIN, OUTPUT);
 
  activeHour = false;
  activeMinute = false;
  alarmActive = false;
  mode = TIME;
}


void loop() {
  dt = clock.getDateTime();
  at = clock.getAlarm1();
  checkMode();
  setLED();
  
  static int time[2] = {0,0};
  getTime(time);

  displayFullTime(time[0], time[1]);
}
