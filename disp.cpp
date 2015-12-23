#include "defines.h"
#include "disp.h"
#include "timed.h"
// LedControl Library for MAX7221 7 segment display handling
#include "LedControl.h"
LedControl disp = LedControl(11, 13, 12, 1); // DataIn (MOSI), CLK (SCK), LOAD (CS), Qty of MAX7221s

#define CdS_Pin A0

void displayStart() {
  // 7 segment display initialization
  disp.setScanLimit(0, 8);  // Limit scanning to 4 digits
  disp.setIntensity(0, 0);  // Set the brightness minimum value on start
  disp.clearDisplay(0);     // Clear the display
  disp.shutdown(0, false);  // Wake up MAX7221 (get out of power saving mode)
  displayTime();
}

void displayBrightness(uint8_t _forcedValue) {
  const uint8_t readCounts = 100;
  static uint8_t sensorCount = 0;
  static uint32_t sensorValue = 0;
  static uint8_t lastBrightnessValue = 0;
  uint8_t brightnessValue = 0;

  if (Run_Mode != RM_ALARM_TRIG) {
    sensorValue = sensorValue + analogRead(CdS_Pin);
    sensorCount++;

    if (sensorCount == readCounts) {  // Averaging
      brightnessValue = ((sensorValue / readCounts) / 64);
      if (brightnessValue - lastBrightnessValue <= 2) {

#ifdef _CDS_DEBUG
        Serial.print(F("brightnessValue :   "));
        Serial.println(brightnessValue);
#endif
        if (readGPIO(BAT_DETECT) == LOW) { // Check BAT_DETECT value (LOW: Power source == battery)
          brightnessValue = 0;
        }
        disp.setIntensity(0, brightnessValue);  // Set the brightness

      }
      lastBrightnessValue = brightnessValue;
      sensorValue = 0;
      sensorCount = 0;
    }
  }
  else if (_forcedValue != NULL){ // We are in alarm triggered mode, so pass the value set by flash directly
#ifdef _CDS_DEBUG
    Serial.print(F("_forcedValue :   "));
    Serial.println(_forcedValue);
    Serial.println();
#endif
    disp.setIntensity(0, _forcedValue);
  }
}

void displayError(uint8_t _errno) {

  switch (_errno) {

    case 42:  // RTC time invalid

      break;
    case 1:  // DS3231 not detected
    case 2:  // MCP23017 not detected
      disp.setChar(0, 0, 'e', false);
      disp.setRow(0, 1, 0x05);  // r
      disp.setRow(0, 2, 0x05);  // r
      disp.setDigit(0, 3, _errno, false);
      break;
    case 3:  // RDA5807M not detected
    case 4:  // DS3909 not detected (temporary device for testing)
      // Figure out what to do
      break;
  }

}


/*
  void displayTemp() {
  int v = -15; // change to sensor reading from DS3231 ?
  uint8_t ones;
  uint8_t tens;
  boolean negative = false;

  if (v < -99 || v > 99)
    return;
  if (v < 0) {
    negative = true;
    v = v * -1;
  }
  ones = v % 10;
  v = v / 10;
  tens = v % 10;

  if (negative) {
    //print character '-' in the leftimeost column
    disp.setChar(0, 0, '-', false);
  }
  else {
    //print a blank in the sign column
    disp.setChar(0, 0, ' ', false);
  }
  //Now print the number digit by digit
  disp.setDigit(0, 1, tens, false);
  disp.setDigit(0, 2, ones, false);

  disp.setChar(0, 3, 'c', false);
  }
*/
uint32_t prevDispMillis = 0;

bool displayNeedsUpdate() {
  uint32_t currentMillis = millis();
  if ((uint32_t)(currentMillis - prevDispMillis) >= disp_update_delay) {
    prevDispMillis = currentMillis;
    return 1;
  }
  else {
    return 0;
  }
}

void displayMode(uint8_t _mode) {


  if (_mode == DISABLED) {
    disp.setChar(0, 0, ' ', 0);
    disp.setChar(0, 1, '0', 0);
    disp.setChar(0, 2, 'F', 0);
    disp.setChar(0, 3, 'F', 0);
  }
  else if (_mode == BEEP) {
    disp.setChar(0, 0, 'B', 0);
    disp.setChar(0, 1, 'E', 0);
    disp.setChar(0, 2, 'E', 0);
    disp.setChar(0, 3, 'P', 0);
  }
  else if (_mode == FM) {
    disp.setChar(0, 0, ' ', 0);
    disp.setRow(0, 1, 0x05);
    disp.setChar(0, 2, 'A', 0);
    disp.setChar(0, 3, 'd', 0);
  }
  else if (_mode == MP3) {
    disp.setChar(0, 0, ' ', 0);
    disp.setChar(0, 1, ' ', 0);
    disp.setDigit(0, 2, 5, 0);
    disp.setChar(0, 3, 'd', 0);
  }
  else if (_mode == BT) {
    disp.setDigit(0, 0, 8, 0);
    disp.setChar(0, 1, 'L', 0);
    disp.setRow(0, 2, 62);
    disp.setChar(0, 3, 'E', 0);
  }
  prevDispMillis = millis();
  disp_update_delay = 500;
}

void displayFrequency(uint16_t _frequency) {

  uint8_t d0, d1, d2, d3;

  _frequency = _frequency / 10;
  d3 = _frequency % 10;
  _frequency = _frequency / 10;
  d2 = _frequency % 10;
  _frequency = _frequency / 10;
  d1 = _frequency % 10;
  _frequency = _frequency / 10;
  d0 = _frequency % 10;

  if (d0 == 0) { // If the first digit is 0
    disp.setChar(0, 0, ' ', false);
  }
  else {
    disp.setDigit(0, 0, d0, false);
  }

  // Now print the frequency digit by digit
  disp.setDigit(0, 1, d1, false);
  disp.setDigit(0, 2, d2, true);
  disp.setDigit(0, 3, d3, false);

  prevDispMillis = millis();
  disp_update_delay = 500;
}

void displayTime() {

  uint8_t time_minutes;
  uint8_t time_hours;
  uint8_t hours_d1;
  uint8_t hours_d2;
  uint8_t minutes_d1;
  uint8_t minutes_d2;

  // Displays Minutes and seconds for debugging purposes

  switch (Run_Mode) {

    case RM_TIME_SET:
    case RM_ALARM_SET:
      time_hours = temp_time[HOUR];
      time_minutes = temp_time[MINUTE];
      break;
    case RM_TIME_DISP:
    case RM_ALARM_TRIG:
      RTC.read(time);
      time_minutes = time.Minute;
      time_hours = time.Hour;
      break;
  }


  // Check for 12/24h
  if (TIMESELECT) { // If we're in 12h mode, set the dot on digit3 if _time_hours is over 12

    if (time_hours > 12) {
      dot_state[3] = true;
      time_hours = time_hours - 12;
    }
    else {
      dot_state[3] = false;
    }
  }// end check for 12/24h


  if (Run_Mode != RM_ALARM_SET) {
    if (alarm_enabled != DISABLED) {
      dot_state[3] = true;
    }
    else {
      dot_state[3] = false;
    }
  }

  hours_d2 = time_hours % 10;
  time_hours = time_hours / 10;
  hours_d1 = time_hours % 10;

  minutes_d2 = time_minutes % 10;
  time_minutes = time_minutes / 10;
  minutes_d1 = time_minutes % 10;

  if (hours_d1 == 0 && TIMESELECT) { // If the first digit is 0 and we're in 12H mode...
    disp.setChar(0, 0, ' ', dot_state[0]);
  }
  else {
    disp.setDigit(0, 0, hours_d1, dot_state[0]);
  }

  // Now print the "time" digit by digit
  disp.setDigit(0, 1, hours_d2, dot_state[1]);
  disp.setDigit(0, 2, minutes_d1, dot_state[2]);
  disp.setDigit(0, 3, minutes_d2, dot_state[3]);

  if (Run_Mode == RM_TIME_SET || Run_Mode == RM_ALARM_SET) {
    disp_update_delay = 0;
  }
  else {
    disp_update_delay = 1000;
  }

  prevDispMillis = millis();
}


