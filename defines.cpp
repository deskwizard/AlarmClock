#include <Arduino.h>    // Required for Serial and delay()
#include <Wire.h>       // I2C library
#include <EEPROM.h>
#include "defines.h"
#include "disp.h"
#include "buttons_react.h"
#include "GPIO.h"
#include "media.h"

extern GPIOHandling GPIO[GPIO_PINS];

void configureGPIO() {
  // Configures Arduino pins as required. MCP pins are handled by the library
  for (uint8_t pin = 0; pin < GPIO_PINS; pin++) {
    if (GPIO[pin].source == Arduino) {
      pinMode(GPIO[pin].pin, GPIO[pin].direction);
      if (GPIO[pin].direction == OUTPUT) {
        digitalWrite(GPIO[pin].pin, GPIO[pin].state);
      }
      else {
        GPIO[pin].state = readGPIO(0);
      }
    }
  }
}

bool readGPIO(uint8_t pin) {
  if (GPIO[pin].source == Arduino) {
    GPIO[pin].state = digitalRead(GPIO[pin].pin);
    return GPIO[pin].state;
  }
}

void setGPIO(uint8_t pin, uint8_t value) {
  /*
    struct GPIOHandling
    {
    uint8_t source; // Arduino Digital (0) or MCP (1)
    uint8_t pin;    // Pin # (see MCP library for pin name convention)
    bool state;     // State of the pin
    uint16_t timeout; // Current timeout value for timed GPIO
    uint32_t prevtime; // Time at which timeout was requested
    };

    GPIOHandling GPIO[8] = {
    { 0, 13, 0, 0, 0}, // GPIO[0]
  */
  bool state;

  Serial.print(F("GPIO Source: "));
  Serial.println(GPIO[pin].source);

  if (GPIO[pin].source == Arduino) {
    Serial.println(F("GPIO Source is Arduino"));
    if (value == INV) {
      GPIO[pin].state = !GPIO[pin].state;
    }
    else if (GPIO[pin].state != value) {
      GPIO[pin].state = value;
    }
    digitalWrite(GPIO[pin].pin, GPIO[pin].state);
  }
  else {
    Serial.print(F("GPIO Source is MCP23017 GPIO "));
    Serial.println(GPIO[pin].pin);
    expanderSetGPIO(GPIO[pin].pin, value);
  }
  Serial.println();
}

// *** BUG: Reset timer if another button is pressed else it will trigger and might change latest set
void setGPIO(uint8_t pin, uint8_t value, uint16_t timeout) {
  GPIO[pin].timeout = timeout;
  GPIO[pin].prevtime = millis();
  setGPIO(pin, value);
}


// Timed events handling

uint32_t prevDispMillis = 0;
uint32_t prevFlashMillis = 0;
uint32_t prevMelodyMillis = 0;
bool alarm_flash_state;

void timedEvents() {

  uint32_t currentMillis = millis();

  // Check for GPIO timeouts

  for (uint8_t x = 0; x < GPIO_PINS; x++) {

    if (((uint32_t)(currentMillis - GPIO[x].prevtime) >= GPIO[x].timeout) && GPIO[x].timeout != 0) {
#ifdef _GPIO_DEBUG
      Serial.print(F("Timeout value for GPIO: "));
      Serial.print(x);
      Serial.print(": ");
      Serial.print(GPIO[x].timeout);
      Serial.println();
      Serial.print(F("Timeout elapsed for GPIO: "));
      Serial.println(x);
      Serial.println();
#endif
      GPIO[x].timeout = 0;
      setGPIO(x, INV);
    }

  }

  //*****************************

  // Display update code - Updates display every second or immediately
  if ((uint32_t)(currentMillis - prevDispMillis) >= disp_update_delay ) {
    displayUpdate();  // Update display
    prevDispMillis = currentMillis;
    if (Run_Mode == RM_TIME_DISP || Run_Mode == RM_ALARM_TRIG) {
      alarmCheck();
    }
  }

  //******************

  // Dot flash code - Flashed DP1
  if ((uint32_t)(currentMillis - prevFlashMillis) >= flash_delay && (Run_Mode == RM_TIME_SET || Run_Mode == RM_ALARM_SET)) {
    if (Run_Mode == RM_TIME_SET) {
      dot_state[1] = !dot_state[1];
    }
    if (Run_Mode == RM_ALARM_SET) {
      dot_state[3] = !dot_state[3];
    }
    prevFlashMillis = currentMillis;
  }

  // Play melody on alarm
  if (!alarm_handled && Run_Mode == RM_ALARM_TRIG && alarm_mode != DISABLED) {
    if (alarm_mode == BEEP) {
      playMelody();
    }
  }

} // end Timed Events


void alarmCheck() {

  // Reset alarm flag
  if ((alarm_time[HOUR] == time.Hour && alarm_time[MINUTE] == (time.Minute + 1)) && alarm_handled && Run_Mode != RM_ALARM_TRIG) {
    alarm_handled = 0;
  }

  // Check for alarm (Will need modifications)
  if ((alarm_time[HOUR] == time.Hour && alarm_time[MINUTE] == time.Minute) && !alarm_handled && Run_Mode != RM_ALARM_TRIG && alarm_mode != 0) {
    Run_Mode = RM_ALARM_TRIG;
    if (alarm_mode == FM) {
      radioPower();
    }
    if (alarm_mode == MP3) {
      mp3Power();
    }
  }
  // *** BUG:  will trigger if time = alarm on power up, but will not beep, so check for alarm_mode ?
  if (!alarm_handled && Run_Mode == RM_ALARM_TRIG) { // If alarm is triggered and not handled...
    // Flash display
    if (alarm_flash_state) {
      displayBrightness(31);
    }
    else {
      displayBrightness(2);
    }
    alarm_flash_state = !alarm_flash_state;

#ifdef _SERIAL_DEBUG
    Serial.println();
    Serial.print(F("Alarm flash triggered: "));
    Serial.println(alarm_flash_state);
    Serial.println();
#endif
  }

}


uint8_t current_tone = 112;
void playMelody() {

  setAudioOut(BEEP); // Set audio output to Arduino beep

  // Need its own millis because its time sensitive
  uint32_t currentMillis = millis();
  if (currentMillis - prevMelodyMillis > 170) {
    prevMelodyMillis = currentMillis;
    tone(SPK_OUT, (2 * current_tone), 160);
    current_tone = current_tone + 12;
    if (current_tone > 236) {
      current_tone = 112;
    }
  }
} // end play Melody


// Check for external device presence (when possible...)
void hwCheck() {

  uint8_t error;
  uint8_t address = 0x68; // Start with RTC

  Wire.beginTransmission(address);
  error = Wire.endTransmission();

  if (error == 0)
  {
#ifdef _SERIAL_DEBUG
    Serial.println(F("RTC Detected"));
#endif
  }
  else
  {
#ifdef _SERIAL_DEBUG
    Serial.println(F(" -- RTC NOT PRESENT! -- "));
#endif
    displayError(0);
  }

  address = 0x20; // MCP23017

  Wire.beginTransmission(address);

  error = Wire.endTransmission();

  if (error == 0)
  {
#ifdef _SERIAL_DEBUG
    Serial.println(F("MCP Detected"));
#endif
  }
  else
  {
#ifdef _SERIAL_DEBUG
    Serial.println(F(" -- MCP NOT PRESENT! -- "));
#endif
    displayError(0);
  }

  address = 0x11; // RDA5807

  Wire.beginTransmission(address);

  error = Wire.endTransmission();

  if (error == 0)
  {
#ifdef _SERIAL_DEBUG
    Serial.println(F("RDA Detected"));
#endif
  }
  else
  {
#ifdef _SERIAL_DEBUG
    Serial.println(F(" -- RDA NOT PRESENT! -- "));
#endif
    displayError(0);
  }
  Serial.println();

}



// write a register value using 2 bytes into the Wire.
void eepromUpdate16(uint8_t _laddr, uint8_t _haddr, uint16_t _value) {
  EEPROM.update(_haddr, _value >> 8);
  EEPROM.update(_laddr, _value & 0xFF);
}


// Read 2 EEPROM addresses values into uint16_t
uint16_t eepromRead16(uint8_t _laddr, uint8_t _haddr) {
  uint8_t hiByte = EEPROM.read(_haddr);
  uint8_t loByte = EEPROM.read(_laddr);
  return (256 * hiByte + loByte);
}




// ********* Debug routines ************

#ifdef  _SERIAL_DEBUG
const char str1[] = "DS1307 RTC Detected - Proceeding...";
const char str2[] = "DS1307 RTC stopped.";
const char str3[] = "Please run the SetTime example to";
const char str4[] = "initialize the time and begin running.";
const char str5[] = "DS1307 read error!  Please check the circuitry.";

void printrtc() {
  Serial.print(time.Year + 1970);
  Serial.print("/");
  Serial.print(time.Month);
  Serial.print("/");
  Serial.print(time.Day);
  Serial.print("  -  ");
  Serial.print(time.Hour);
  Serial.print(":");
  Serial.print(time.Minute);
  Serial.print(":");
  Serial.println(time.Second);
  Serial.println();
}

void print2digits(uint8_t number) {
  if (number >= 0 && number < 10) {
    Serial.print(F("0"));
  }
  Serial.print(number);
}

void debug_start() {
  Serial.begin(9600);
  while (!Serial) ; // wait for serial
  delay(200);
  Serial.println(F("-- Alarm Clock sketch begin--"));
  Serial.println();
}
#ifdef _RTC_DEBUG
void rtc_debug() {

  if (RTC.read(time)) {
    Serial.print(F("Ok, Time = "));
    print2digits(time.Hour);
    Serial.write(F(':'));
    print2digits(time.Minute);
    Serial.write(F(':'));
    print2digits(time.Second);
    Serial.print(F(", Date (D/M/Y) = "));
    Serial.print(time.Day);
    Serial.write(F('/'));
    Serial.print(time.Month);
    Serial.write(F('/'));
    Serial.print(tmYearToCalendar(time.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.println(str2);
      Serial.println(str3);
      Serial.println(str4);
      Serial.println();
      Serial.println();
    } else {
      Serial.println(str5);
      Serial.println();
    }
    delay(10000);
  }

  Serial.println(sensorValue);
}
#endif //end rtc debug
#endif // end serial debug
// ********* END debug routines ************
