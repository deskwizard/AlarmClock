#ifndef _TIMED_H
#define _TIMED_H

// Timed routines
#define flash_delay 500
extern uint16_t disp_update_delay;

// Timed GPIO
#define INV 2 // LOW = 0, HIGH = 1, INV = 2

// Externs

// Run mode handling
extern uint8_t Run_Mode;
extern bool alarm_handled;
extern uint8_t alarm_type;
extern bool alarm_enabled;

// Routines
void i2cCheck();
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
