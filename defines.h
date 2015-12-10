#ifndef _DEFINES_H_
#define _DEFINES_H_
#include <Arduino.h>

// Debugging
#define _SERIAL_DEBUG     // Serial debugging
//#define _RTC_DEBUG        // RTC debugging
//#define _CDS_DEBUG     // Serial debugging

#define DISABLED 0
#define ENABLED 1

// Alarm
#define BEEP 1  // only for alarm
#define FM   2
#define MP3  3
#define BT 4

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

extern uint8_t alarm_time[2];

#endif
