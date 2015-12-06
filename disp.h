#ifndef _FAIL_H_
#define _FAIL_H_

//#include "LedControl.h" // MAX7221 Library
//#include <Wire.h> // I2C library
#include <Time.h> // Time library
#include <DS1307RTC.h>  // DSxxxx Library

#define BAT_DETECT 2 // battery detect on GPIO[2] (Arduino pin 8, See GPIO.h)

extern tmElements_t time; // Time library element (required)

extern uint8_t Run_Mode;
extern uint8_t TIMESELECT;
extern uint8_t temp_time[3];
extern uint16_t temp_date[3];
extern bool dot_state[4];
extern uint8_t alarm_mode;
extern uint16_t disp_update_delay;

void displayStart();
void displayMode(uint8_t _mode);
void displayError(uint8_t _errno);
void displayBrightness(uint8_t _value);
void displayAutoBrightness();
void displayUpdate();
void displayFrequency(uint16_t _frequency);
//void displayTemp();

#endif
