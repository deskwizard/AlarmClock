#ifndef _GPIO_H
#define _GPIO_H
#include "Arduino.h"

#define GPIO_PINS 11 // Number of GPIO pins

// GPIO sources
#define Arduino 0
#define MCP 1

struct GPIOHandling
{
  uint8_t source; // Arduino Digital (0) or MCP (1)
  uint8_t pin;    // Pin # (see MCP library for pin name convention)
  uint8_t direction; // Input or Output
  bool state;     // State of the pin
  uint16_t timeout; // Current timeout value for timed GPIO
  uint32_t prevtime; // Time at which timeout was requested
};


// GPIO outputs

// Speaker output and power source input should be switched.... (7 and 8)
#define SPK_OUT 7

// GPIO[0] = 0 (Arduino), 5 (Pin 5), INPUT, 0 (Default state), 0 (timeout), 0 (Timeout request time)
// GPIO[3] = 1 (MCP23017), 0 (GPIO0), OUTPUT,  0 (Default state), 0 (timeout), 0 (Timeout request time)

GPIOHandling GPIO[GPIO_PINS] = {
  { 0, 5, OUTPUT, 0, 0, 0}, // GPIO[0] = 0 (Arduino), 5 (Pin 5), INPUT, 0 (Default state), 0 (timeout), 0 (Timeout request time)
  { 0, 6, OUTPUT, 0, 0, 0}, // GPIO[1] = 0 (Arduino), 6 (Pin 6), INPUT, 0 (Default state), 0 (timeout), 0 (Timeout request time)
  { 0, 8, INPUT_PULLUP, 0, 0, 0}, // GPIO[2] = 0 (Arduino), 8 (Pin 8), INPUT_PULLUP, 0 (Default state), 0 (timeout), 0 (Timeout request time)
  { 1, 0, OUTPUT, 0, 0, 0},  // GPIO[3] =
  { 1, 1, OUTPUT, 0, 0, 0}, // etc...
  { 1, 2, OUTPUT, 0, 0, 0},
  { 1, 3, OUTPUT, 0, 0, 0}, // GPIO[6] = MP3 Prev
  { 1, 4, OUTPUT, 0, 0, 0}, // GPIO[6] = MP3 Next
  { 1, 5, OUTPUT, 0, 0, 0}, // GPIO[8] = MP3 Vol -
  { 1, 6, OUTPUT, 0, 0, 0}, // GPIO[9] = MP3 Vol +
  { 1, 7, OUTPUT, 0, 0, 0}, // GPIO[10] = MP3_POWER
};




#endif
