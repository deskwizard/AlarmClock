#include "defines.h" // Defines
#include "timed.h" // timed events/routines 
#include "disp.h" // Display routines
#include "buttons_react.h"

#include <EEPROM.h> // Temporary for saving alarm 

// Time and DS1307 Library for timekeeping
#include <Wire.h> // I2C library
#include <DS1307RTC.h>  // DSxxxx Library
#include <Time.h> // Time library (required)
tmElements_t time; // Time library element (required)

uint8_t TIMESELECT = _24H;
uint8_t Run_Mode = RM_TIME_DISP;
uint8_t Media_Mode = DISABLED;
uint8_t temp_time[3] = {0};
uint16_t temp_date[3] = {0};
bool dot_state[4] = {0};
uint16_t disp_update_delay = 1000;

// To be replace with interrupt and DS3231 alarm register
uint8_t alarm_time[2] = {0, 0}; // Run_Mode = RM_ALARM_TRIG -  (Hour, Minute)
bool alarm_handled = 0;
uint8_t alarm_mode = 1; // Default to beep
bool alarm_enabled = 0;

// **** Start setup() ****
void setup() {
  // Get Alarm value (read eeprom value until DS3231 gets here)
  alarm_time[HOUR] = EEPROM.read(HOUR);
  alarm_time[MINUTE] = EEPROM.read(MINUTE);

#ifdef _SERIAL_DEBUG
  debug_start();
#endif
  TWBR = (F_CPU / 24992);
  configureGPIO();
  hwCheck();           // Check for external device presence (whenever possible)
  expanderStart();
  displayStart();         // 7 segment display initialization

}  // **** End setup() ****



// **** Start loop() ****
void loop() {

  // Automatic brightness
  displayAutoBrightness();

  //  Button handling
  expanderButtonReact();

  // Timed events (Flashes, Timed-GPIO, etc...)
  timedEvents();

  // RTC debug
#ifdef _RTC_DEBUG
  rtc_debug();
#endif

}// **** End loop() ****
