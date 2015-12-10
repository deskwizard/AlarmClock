#ifndef _BUTTONS_REACT_H
#define _BUTTONS_REACT_H
#include <Arduino.h>

#include <Time.h> // Time library
#include <DS1307RTC.h>  // DSxxxx Library

extern tmElements_t time; // Time library element (required)

void expanderStart();
void expanderButtonReact();
void expanderSetGPIO(uint8_t pin, uint8_t value);
/*
  Types of button clicks
  #define MCP_PRESS      1  // ID for button press
  #define MCP_CLICK     2  // ID for clicks (also button release)
  #define MCP_DOUBLE_CLICK  3    // ID for double clicks
  #define MCP_LONG_CLICK    4    // ID for long clicks
  #define MCP_HELD_DOWN     5    // ID for button held down
  #define MCP_HELD_RELEASE  6    // ID for button held down

  Run modes:
  #define RM_TIME_DISP 0
  #define RM_TIME_SET 1
  #define RM_ALARM_SET 2
  #define RM_ALARM_TRIG 3
*/
extern uint8_t Run_Mode;
extern uint8_t TIMESELECT;
extern uint8_t temp_time[3];
extern uint16_t temp_date[3];
extern bool dot_state[4];
extern bool alarm_handled;
extern uint8_t Media_Mode;

#endif
