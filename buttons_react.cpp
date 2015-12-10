#include "buttons_react.h"
#include "defines.h"
#include "disp.h"
#include "media.h"
#include "timed.h"

#include <EEPROM.h> // Temporary for saving alarm 
#include <MCP23017.h>           // Include MCP23017 library

// Starts I2C bus for MCP on address 'address', prepare INT(which) for 'button_count' buttons
// and configure 'output_count' outputs
MCP23017 expander(0x20, 0, 8, 8);
/*
  Types of button clicks (expander.buttonClickType[x]
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

void expanderStart() {
  expander.begin();     // Start expander
}

void expanderSetGPIO(uint8_t pin, uint8_t value) {
  Serial.println(F("expanderSetGPIO"));
  Serial.println();
  expander.setGPIO(pin, value);
}

void expanderButtonReact() {

  // Take care of the clicks....
  expander.handleClicks();

  // ... then react to them....

  // You can only read the button state once, so read it into a variable and do the checks on that.
  uint8_t button_read[8];

  for (uint8_t x = 0; x < 8; x++) {  // loop through buttons to read the events
    button_read[x] = expander.buttonRead(x);
  }

  if (Run_Mode == RM_ALARM_TRIG) {
    for (uint8_t x = 0; x < 8; x++) {  // loop through buttons to read the events
      if (is_new_of_type(button_read[x], MCP_CLICK)) { // if alarm is triggered any button click turns it off, and button click gets ignored
        alarm_handled = 1;
        setAudioOut(DISABLED); // Set audio output disabled.
        if (alarm_mode == FM) {
          radioPower();
        }
        if (alarm_mode == MP3) {
          mp3Power();
        }
        Run_Mode = RM_TIME_DISP;
        return;
      }
    }
  }


  // ****** React according to Run_Mode value ******

  // Time display mode (default)
  if (Run_Mode == RM_TIME_DISP) {

    // Button 0 - Held Down - RM_TIME_DISP -> Enter time set mode
    if (is_new_of_type(button_read[0], MCP_LONG_CLICK) && Media_Mode == DISABLED) {
      temp_time[HOUR] = time.Hour;
      temp_time[MINUTE] = time.Minute;
      temp_time[SECOND] = time.Second;
      Run_Mode = RM_TIME_SET;
#ifdef _SERIAL_DEBUG
      Serial.println(F("Run_Mode RM_TIME_SET"));
      Serial.print(F("Media mode: "));
      Serial.println(Media_Mode);
      Serial.print(time.Hour);
      Serial.print(F(":"));
      Serial.print(time.Minute);
      Serial.print(F(":"));
      Serial.println(time.Second);
#endif
      return;
    }

    // Button 5 - Held Down - RM_TIME_DISP -> Enter alarm set mode
    if (is_new_of_type(button_read[5], MCP_LONG_CLICK) && Media_Mode == DISABLED) {
      temp_time[HOUR] = alarm_time[HOUR];
      temp_time[MINUTE] = alarm_time[MINUTE];
      Run_Mode = RM_ALARM_SET;
      displayUpdate();
#ifdef _SERIAL_DEBUG
      Serial.println(F("Run_Mode RM_Alarm_SET"));
      Serial.print(temp_time[HOUR]);
      Serial.print(F(":"));
      Serial.println(temp_time[MINUTE]);
#endif
      return;
    }

    // Button 5 - Click - RM_TIME_DISP -> Alarm enable/Disable
    if (is_new_of_type(button_read[5], MCP_CLICK) && Media_Mode == DISABLED) {

      alarm_enabled = !alarm_enabled;

#ifdef _SERIAL_DEBUG
      Serial.println(F("Alarm enabled: "));
      Serial.println(alarm_enabled);
#endif
    }

    // ****** Media handling buttons ******


    if (is_new_of_type(button_read[0], MCP_CLICK)) {
      mediaPower();
#ifdef _SERIAL_DEBUG
      Serial.print(F("Media Power"));
      Serial.println();
#endif
    }
    if (Media_Mode != DISABLED) {
      if (is_new_of_type(button_read[0], MCP_LONG_CLICK)) {
        mediaChange();
#ifdef _SERIAL_DEBUG
        Serial.print(F("Media Change"));
        Serial.println();
#endif
      }


      if (is_new_of_type(button_read[3], MCP_CLICK)) {
        setVolume(DOWN);
#ifdef _SERIAL_DEBUG
        Serial.print(F("Vol-"));
        Serial.println();
#endif
      }
      if (is_new_of_type(button_read[4], MCP_CLICK)) {
        setVolume(UP);
#ifdef _SERIAL_DEBUG
        Serial.print(F("Vol+"));
        Serial.println();
#endif
      }


      if (Media_Mode == FM) {  // If FM is active
        if (is_new_of_type(button_read[1], MCP_CLICK)) {
          radioSeek(DOWN);
#ifdef _SERIAL_DEBUG
          Serial.print(F("Radio seek down"));
          Serial.println();
#endif
        }
        if (is_new_of_type(button_read[2], MCP_CLICK)) {
          radioSeek(UP);
#ifdef _SERIAL_DEBUG
          Serial.print(F("Radio seek up"));
          Serial.println();
#endif
        }
      }
      else if (Media_Mode == MP3) { // If MP3 is active

        // ********* MP3 module Next/Prev/FF/Rew handling ************
        if (is_new_of_type(button_read[1], MCP_CLICK)) {
          mp3Prev();
#ifdef _SERIAL_DEBUG
          Serial.print(F("MP3 Previous"));
          Serial.println();
#endif
        }
        if (is_new_of_type(button_read[2], MCP_CLICK)) {
          mp3Next();
#ifdef _SERIAL_DEBUG
          Serial.print(F("MP3 Next"));
          Serial.println();
#endif
        }
        if (is_new_of_type(button_read[1], MCP_HELD_DOWN)) {
          mp3Rew(1);
#ifdef _SERIAL_DEBUG
          Serial.print(F("MP3 Rew start"));
          Serial.println();
#endif
        }
        if (is_new_of_type(button_read[2], MCP_HELD_DOWN)) {
          mp3FF(1);
#ifdef _SERIAL_DEBUG
          Serial.print(F("MP3 FF start"));
          Serial.println();
#endif
        }
        if (is_new_of_type(button_read[1], MCP_HELD_RELEASE)) {
          mp3Rew(0);
#ifdef _SERIAL_DEBUG
          Serial.print(F("MP3 Rew stop"));
          Serial.println();
#endif
        }
        if (is_new_of_type(button_read[2], MCP_HELD_RELEASE)) {
          mp3FF(0);
#ifdef _SERIAL_DEBUG
          Serial.print(F("MP3 FF stop"));
          Serial.println();
#endif
        }

        else if (stateBT) {

        }


        // *** BUTTONS FOR DEBUG

      }
    } // End Media_Mode != DISABLED
  } // End Run mode RM_TIME_DISP



  //  ***************** Time / Alarm set mode *************************
  if (Run_Mode == RM_TIME_SET || Run_Mode == RM_ALARM_SET) {


    // Button 0 - Long click - RM_TIME_SET -> Reset seconds, Save new time, Exit time set mode
    if (is_new_of_type(button_read[0], MCP_LONG_CLICK) && Run_Mode == RM_TIME_SET) {
      // Reset seconds
      time.Hour = temp_time[HOUR];
      time.Minute = temp_time[MINUTE];
      time.Second = 0;

      RTC.write(time); // Save the new time
#ifdef _SERIAL_DEBUG
      Serial.println(F("Saving time (and setting seconds to 0)"));
      printrtc(); // Prints date and time
#endif

      dot_state[1] = 0; // Turn off dot period that was flashing
      Run_Mode = RM_TIME_DISP;
      displayUpdate();
    }


    // Button 5 - Long click - RM_ALARM_SET -> Save new alarm time, Exit alarm set mode
    if (is_new_of_type(button_read[5], MCP_LONG_CLICK) && Run_Mode == RM_ALARM_SET) {

      alarm_time[HOUR] = temp_time[HOUR];
      alarm_time[MINUTE] = temp_time[MINUTE];

      // Save new alarm value (temporarily use eeprom)
      EEPROM.update(HOUR, alarm_time[HOUR]);
      EEPROM.update(MINUTE, alarm_time[MINUTE]);

#ifdef _SERIAL_DEBUG
      Serial.print(F("Saving Alarm time: "));
      Serial.print(alarm_time[HOUR]);
      Serial.print(F(":"));
      Serial.println(alarm_time[MINUTE]);
      Serial.println();
#endif
      dot_state[3] = 0; // Turn off dot period that was flashing
      Run_Mode = RM_TIME_DISP;
      displayUpdate();
    }


    // Button 5 - Click - RM_ALARM_SET -> Change alarm mode
    if (is_new_of_type(button_read[5], MCP_CLICK) && Run_Mode == RM_ALARM_SET) {

      alarm_mode++;
      if (alarm_mode > MP3) {
        alarm_mode = 1;
      }
      displayMode(alarm_mode);
#ifdef _SERIAL_DEBUG
      Serial.println(F("Alarm mode change:"));
      Serial.println(alarm_mode);
#endif
    }


    // Value change buttons

    if (is_new_of_type(button_read[1], MCP_CLICK) || is_new_of_type(button_read[1], MCP_DOUBLE_CLICK)) {
      --temp_time[HOUR];
      if (temp_time[HOUR] > 23) {
        temp_time[HOUR] = 23;
      }
#ifdef _SERIAL_DEBUG
      Serial.print(F("Hours (-): "));
      Serial.println(temp_time[HOUR]);
      Serial.println();
#endif
    }
    if (is_new_of_type(button_read[2], MCP_CLICK) || is_new_of_type(button_read[2], MCP_DOUBLE_CLICK)) {
      temp_time[HOUR]++;
      if (temp_time[HOUR] > 23) {
        temp_time[HOUR] = 0;
      }
#ifdef _SERIAL_DEBUG
      Serial.print(F("Hours (+): "));
      Serial.println(temp_time[HOUR]);
      Serial.println();
#endif
    }
    if (is_new_of_type(button_read[3], MCP_CLICK) || is_new_of_type(button_read[3], MCP_DOUBLE_CLICK)) {
      --temp_time[MINUTE];
      if (temp_time[MINUTE] > 59) {
        temp_time[MINUTE] = 59;
      }
#ifdef _SERIAL_DEBUG
      Serial.print(F("Minutes (-): "));
      Serial.println(temp_time[MINUTE]);
      Serial.println();
#endif
    }
    if (is_new_of_type(button_read[4], MCP_CLICK) || is_new_of_type(button_read[4], MCP_DOUBLE_CLICK)) {
      temp_time[MINUTE]++;
      if (temp_time[MINUTE] > 59) {
        temp_time[MINUTE] = 0;
      }
#ifdef _SERIAL_DEBUG
      Serial.print(F("Minutes (+): "));
      Serial.println(temp_time[MINUTE]);
      Serial.println();
#endif
    }
  } //end Run_Mode == RM_TIME_SET/RM_ALARM_SET
}
