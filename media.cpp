#include <Arduino.h>
#include <EEPROM.h>
#include "defines.h"
#include "media.h"
#include "disp.h"
#include "timed.h"
#include <RDA5807M.h>

RDA5807M radio;
RADIO_INFO radioInfo;
char sFreq[12]; // frequency string
uint16_t savedFreq;

uint8_t lastMedia = EEPROM.read(LASTMEDIA_ADDR);
uint8_t volume = EEPROM.read(VOLUME_ADDR);

void mediaChange() {

  if (Media_Mode != DISABLED) { // if something is on
    if (Media_Mode == FM) {
      // turn off radio
      radioPower();
      // turn on mp3 module
      mp3Power();
      lastMedia = MP3;
    }
    else if (Media_Mode == MP3) {
      // turn off mp3 module
      mp3Power();
      // turn on BT (temporary for debug)
      Serial.println(F("Turn on BT"));
      displayMode(BT);
      Media_Mode = BT;
      // lastMedia = BT; // Still not sure about BT
    }
    else if (Media_Mode == BT) {
      // turn off BT (temporary for debug)
      Serial.println(F("Turn off BT"));
      Media_Mode = DISABLED; // debug while no bt
      // turn on radio
      radioPower();
      lastMedia = FM;
    }
  }
#ifdef _SERIAL_DEBUG
  Serial.print(F("lastMedia: "));
  Serial.println(lastMedia);
  Serial.println();
  Serial.print(F("Media mode: "));
  Serial.println(Media_Mode);
#endif
  EEPROM.update(LASTMEDIA_ADDR, lastMedia);
}

void mediaPower() {

  if (Media_Mode == DISABLED) { // if everything is off
    // use lastMedia to start the last used media source (not sure what to do with BT...)
    if (lastMedia == FM) {
      // turn on radio
      radioPower();
    }
    else if (lastMedia == MP3) {
      // turn on MP3 module
      mp3Power();
    }
  }
  else { // something is on
    if (Media_Mode = FM) {
      // turn off radio
      radioPower();
    }
    else if (Media_Mode = MP3) {
      // turn off mp3 module
      mp3Power();
    }
    else if (Media_Mode = BT) {
      // turn off BT
      Serial.println(F("power - Turn off BT"));
      Media_Mode = DISABLED;
    }
    displayMode(DISABLED);
    // Save volume
    EEPROM.update(VOLUME_ADDR, volume);
  }

#ifdef _SERIAL_DEBUG
  Serial.print(F("Media mode: "));
  Serial.println(Media_Mode);
#endif

}

void setVolume(bool _direction) {

  if (_direction) { // volume up
    volume = volume + 4;
    if (volume > 124) {
      volume = 124;
    }
  }
  else { // volume down
    volume = volume - 4;
    if (volume > 127) {
      volume = 0;
    }
  }

  Wire.beginTransmission(0x50); // transmit to address 0x50
  Wire.write(0xF9);            // Pot0
  Wire.write(volume);          // sends pot0 value byte
  Wire.endTransmission();     // stop transmitting

#ifdef _SERIAL_DEBUG
  Serial.print(F("Write volume: "));
  Serial.println(volume);
#endif
}

void mp3Power() {
  if (Media_Mode == MP3) {
    // turn off
    setGPIO(MP3_POWER, LOW);
    setAudioOut(DISABLED);
    Media_Mode = DISABLED;
  }
  else {
    // turn on
    setGPIO(MP3_POWER, HIGH);
    setAudioOut(MP3);
    Media_Mode = MP3;
    if (Run_Mode != RM_ALARM_TRIG) {
      displayMode(MP3);
    }
  }

}

void mp3Next() {
  setGPIO(MP3_NEXT, HIGH, 5);
}
void mp3Prev() {
  setGPIO(MP3_PREV, HIGH, 5);
}

void mp3FF(bool _start) {
  if (_start) {
    setGPIO(MP3_NEXT, HIGH);
  }
  else {
    setGPIO(MP3_NEXT, LOW);
  }
}
void mp3Rew(bool _start) {
  if (_start) {
    setGPIO(MP3_PREV, HIGH);
  }
  else {
    setGPIO(MP3_PREV, LOW);
  }
}

void radioPower() {
  // Powers up the radio if its powered down, power it down if its powered up

  if (Media_Mode == FM) {
    Serial.println(radio.getFrequency());
    savedFreq = radio.getFrequency(); // Save currently tuned frequency
    eepromUpdate16(FMFREQ_LADDR, FMFREQ_HADDR, savedFreq);
    radio.term(); // Turn off RDA
    setAudioOut(DISABLED);
    Media_Mode = DISABLED;
  }
  else {
    Media_Mode = FM;
    radio.init();
    radio.setBand(RADIO_BAND_FM);
    radio.setBassBoost(true);
    setAudioOut(FM);
    radio.setMute(false);
    savedFreq = eepromRead16(FMFREQ_LADDR, FMFREQ_HADDR);
    radio.setFrequency(savedFreq);
    radio.setVolume(2); // Initial radio volume, adjust for sources balance
    setAudioOut(FM);
#ifdef _RADIO_DEBUG
    radioDebug();
#endif
    // add if alarm is not triggered for freq display
    if (Run_Mode != RM_ALARM_TRIG) {
      displayFrequency(radio.getFrequency());
    }
  }

}

void radioSeek(bool _direction) {
  if (_direction == UP) {
    radio.seekUp(true);
  }
  else {
    radio.seekDown(true);
  }
  delay(300); // wait for seek (hackey, should check rda but its not in library...)
  displayFrequency(radio.getFrequency());
#ifdef _RADIO_DEBUG
  radioDebug();
#endif
}

void setAudioOut(uint8_t audio_out) {
  // change to switch case?
  if (audio_out == DISABLED) {
    // Disable amplifier & 4051
    setGPIO(AUDIO_EN, LOW);
  }
  else {
    // Enable amplifier & 4051
    setGPIO(AUDIO_EN, HIGH);
    if (audio_out == BEEP) { // Y0
      setGPIO(S1_4051, LOW);
      setGPIO(S2_4051, LOW);
    }
    if (audio_out == FM) {  // Y1
      setGPIO(S1_4051, HIGH);
      setGPIO(S2_4051, LOW);
    }
    if (audio_out == MP3) { // Y2
      setGPIO(S1_4051, LOW);
      setGPIO(S2_4051, HIGH);
    }
    if (audio_out == BT) {  // Y3
      setGPIO(S1_4051, HIGH);
      setGPIO(S2_4051, HIGH);
    }
  }
}

#ifdef _RADIO_DEBUG
void radioDebug() {
  delay(300);
  radio.getRadioInfo(&radioInfo);
  radio.formatFrequency(sFreq, sizeof(sFreq));

  if (radioInfo.tuned) {
    Serial.print("Tuned ");
  }
  Serial.print("Station: ");
  Serial.print(sFreq);
  Serial.print(" (RSSI: ");
  Serial.print(radioInfo.rssi);
  if (radioInfo.rds) {
    Serial.print(" RDS ON");
  }
  Serial.println(")");

  Serial.print("Radio:"); radio.debugRadioInfo();
  Serial.println();

}
#endif
