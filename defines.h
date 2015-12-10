#ifndef _DEFINES_H_
#define _DEFINES_H_

// Debugging
#define _SERIAL_DEBUG     // Serial debugging
//#define _RTC_DEBUG        // RTC debugging
//#define _CDS_DEBUG     // Serial debugging
//#define DEBUG_LED 13

// Mode handling
#define RM_TIME_DISP 0
#define RM_TIME_SET 1
#define RM_ALARM_SET 2
#define RM_ALARM_TRIG 3

// Hour display time base
#define _12H 1
#define _24H 0

#define YEAR 0
#define MONTH 1
#define DAY 2
#define HOUR 0
#define MINUTE 1
#define SECOND 2

// Timed routines
#define flash_delay 500
extern uint16_t disp_update_delay;


#define DISABLED 0
#define ENABLED 1

// Alarm
#define BEEP 1  // only for alarm
#define FM   2
#define MP3  3
#define BT 4

// Timed GPIO
#define INV 2 // LOW = 0, HIGH = 1, INV = 2

// Externs

// Run mode handling
extern uint8_t Run_Mode;

extern uint8_t alarm_time[2];
extern bool alarm_handled;
extern uint8_t alarm_type;
extern bool alarm_enabled;

// Routines
void hwCheck();
void timedEvents();
void alarmCheck();
void playMelody();
void configureGPIO();
bool readGPIO(uint8_t pin);
void setGPIO(uint8_t pin, uint8_t value);
void setGPIO(uint8_t pin, uint8_t value, uint16_t timeout);
uint16_t eepromRead16(uint8_t _laddr, uint8_t _haddr);
void eepromUpdate16(uint8_t _laddr, uint8_t _haddr, uint16_t _value);

// Serial debugging
#ifdef _SERIAL_DEBUG
void rtc_debug();
void debug_start();
void printrtc();
void print2digits(uint8_t number);
#endif
#endif
