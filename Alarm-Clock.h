#include <stdbool.h>
#include <Wire.h>
#include <DS3231.h>

// PINS************
const int BD1 = 2;
const int BD2 = 3;

const int HOURPIN = 6;
const int MINPIN = 5;
const int MODEPIN = 4;

const int DATAPIN = 7;
const int LATCHPIN = 8;
const int CLOCKPIN = 9;
const int ALARMPIN = A1;
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
bool alarmActive;
Modes mode;
int alarmHour;
int alarmMinute;
typedef void (*timeFunction)(int,int);
timeFunction currentCallback;
// VARIABLES OVER**

// FUNCTION *******


void sevenSegDisplay(int);
void updateNumOnDigit(int, int);
void displayFullTime(int, int);
void checkAlarm();
void checkMode();
void setClock(int, int);
void setAlarm(int, int);
void setTime(int, int);

void setup();
void loop();



// FUNCTIONS OVER**


