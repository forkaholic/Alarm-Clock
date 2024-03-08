#include <stdbool.h>
#include <Wire.h>
#include <DS3231.h>

// PINS************
const int D1 = 10;
const int D2 = 11;
const int D3 = 12;
const int D4 = 13;

const int HOUR = 6;
const int MIN = 7;
const int MODE = 3;

const int DATA = 2;
const int LATCH = 4;
const int CLOCK = 5;
const int ALARMPIN = A0;
// PINS OVER*******

// OBJECTS*********
DS3231 clock;
RTCDateTime dt;
// OBJECTS OVER****

// ENUMS & DATA****
enum Modes {
  TIME,
  SET,
  ALARM,
  NUM_MODES
};
int digits[4] = {D1, D2, D3, D4};
int values[4] = {0, 0, 0, 0};
int numbers[11] = {
  0b11111100, // 0
  0b01100000, // 1
  0b11011010, // 2
  0b11110010, // 3
  0b01100110, // 4
  0b10110110, // 5
  0b10111110, // 6
  0b11100000, // 7
  0b11111110, // 8
  0b11100110, // 9
  0b00000000 // Empty for leading zero in hour
};
// ENUMS DATA OVER*

// VARIABLES*******
bool activeHour;
bool activeMinute;
bool activeMode;
int last;
Modes mode;
int alarmHour;
int alarmMinute;
typedef void (*timeFunction)(int,int);
timeFunction currentCallback;
// VARIABLES OVER**

/*
  Send out correct binary number using the data, clock,
  and latch.
*/
void sevenSegDisplay(int num) {
  digitalWrite(LATCH, LOW);
  shiftOut(DATA, CLOCK, LSBFIRST, numbers[num]);
  digitalWrite(LATCH, HIGH);
}

/*
  Turn off the previous digit and set up the next digit.
*/
void updateNumOnDigit(int onPin, int offPin, int num) {
  digitalWrite(offPin, LOW);
  sevenSegDisplay(num);
  digitalWrite(onPin, HIGH);
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
    updateNumOnDigit(digits[i], digits[last], values[i]);
    last = i;
    delay(5);
  }
}

/*
  Check to see if mode button has been pressed,
  if so update accordingly.
*/
void checkMode() {
  if(!activeMode && digitalRead(MODE) == LOW) {
    Serial.println("mode changed");
    Serial.println(mode);
    mode = (Modes)(((int)mode + 1) % (int)NUM_MODES);
    activeMode = true;
    Serial.println(mode);
  }
  else if(activeMode && digitalRead(MODE) == HIGH) {
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
  if(!activeHour && digitalRead(HOUR) == LOW) {
    currentCallback((hour + 1) % 24, minute);
    activeHour = true;
  }
  else if(activeHour && digitalRead(HOUR) == HIGH) {
    activeHour = false;
  }

  if(!activeMinute && digitalRead(MIN) == LOW) {
    currentCallback(hour, (minute + 1) % 60);
    activeMinute = true;
  }
  else if(activeMinute && digitalRead(MIN) == HIGH) {
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


  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(MIN, INPUT_PULLUP);
  pinMode(HOUR, INPUT_PULLUP);
  pinMode(MODE, INPUT_PULLUP);
  pinMode(ALARM, OUTPUT);

  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
  // digitalWrite(ALARM, HIGH);
  digitalWrite(LATCH, HIGH);
  
  activeHour = false;
  activeMinute = false;
  last = 0;
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

  if(dt.hour == alarmHour && dt.minute == alarmMinute) {
    analogWrite(ALARMPIN, 255);
  }
  else {
    analogWrite(ALARMPIN, 0);
  }
}
