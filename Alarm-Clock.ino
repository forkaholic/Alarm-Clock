#include "Alarm-Clock.h";

/*
  Send out correct binary number using the data, clock,
  and latch.
*/
void sevenSegDisplay(int num) {
  digitalWrite(LATCHPIN, LOW);
  shiftOut(DATAPIN, CLOCKPIN, LSBFIRST, numbers[num]);
  digitalWrite(LATCHPIN, HIGH);
}

/*
  Turn off the previous digit and set up the next digit.
*/
void updateNumOnDigit(int digit, int num) {
  digitalWrite(BD1, LOW);
  digitalWrite(BD2, LOW);
  sevenSegDisplay(num);
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
    updateNumOnDigit(i, values[i]);
    delay(5);
  }
}

/*
  Check to see if alarm is to be turned off by reading mode pin
*/
void checkAlarm() {
  if(alarmActive && digitalRead(MODEPIN) == HIGH) {
    alarmActive = false;
  }
}

/*
  Check to see if mode button has been pressed,
  if so update accordingly.
*/
void checkMode() {
  checkAlarm();
  if(alarmActive) {
    return;
  }
  
  if(!activeMode && digitalRead(MODEPIN) == LOW) {
    mode = (Modes)(((int)mode + 1) % (int)NUM_MODES);
    activeMode = true;
    Serial.println(mode);
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
  alarmHour = hour;
  alarmMinute = minute;
}

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
  pinMode(DATAPIN, OUTPUT);
  pinMode(LATCHPIN, OUTPUT);
  pinMode(CLOCKPIN, OUTPUT);
  pinMode(MINPIN, INPUT_PULLUP);
  pinMode(HOURPIN, INPUT_PULLUP);
  pinMode(MODEPIN, INPUT_PULLUP);
  pinMode(ALARMPIN, OUTPUT);

  digitalWrite(ALARMPIN, HIGH);
  digitalWrite(LATCHPIN, HIGH);
  
  activeHour = false;
  activeMinute = false;
  mode = TIME;
  alarmHour = 0;
  alarmMinute = 0;
}


void loop() {
  
  checkMode();
  
  int hour = 0;
  int minute = 0;
  dt = clock.getDateTime();

  switch(mode) {
    case ALARM:
      currentCallback = setAlarm;
      setTime(alarmHour, alarmMinute);
      hour = alarmHour;
      minute = alarmMinute;
      break;

    case SET:
      currentCallback = setClock;
      setTime(dt.hour, dt.minute);
    case TIME:
    default:
      hour = dt.hour;
      minute = dt.minute;
      break;

  }

  displayFullTime(hour, minute);

  // Turn on alarm and force mode to time to display the time, alarm button now turns off alarm
  if(dt.hour == alarmHour && dt.minute == alarmMinute && dt.second == 0) {
    analogWrite(ALARMPIN, 255);
    alarmActive = true;
    mode = TIME;
  }
  else {
    analogWrite(ALARMPIN, 0);
    alarmActive = false;
  }
}
