#include <Arduino.h>
#include <EEPROM.h>
#include "media.h"
#include "defines.h"
#include "disp.h"
#include <RDA5807M.h>

RDA5807M radio;
RADIO_INFO radioInfo;
char sFreq[12]; // frequency string
uint16_t savedFreq;

uint8_t lastMedia = EEPROM.read(LASTMEDIA_ADDR);

void mediaChange() {

  if ((stateFM + stateMP3 + stateBT) > 0) { // if something is on
    if (stateFM == 1) {
      // turn off radio
      radioPower();
      // turn on mp3 module
      mp3Power();
      lastMedia = MP3;
    }
    else if (stateMP3 == 1) {
      // turn off mp3 module
      mp3Power();
      // turn on BT (temporary for debug)
      Serial.println(F("Turn on BT"));
      stateBT = 1;
      displayMode(BT);
      // lastMedia = BT; // Still not sure about BT
    }
    else if (stateBT == 1) {
      // turn off BT (temporary for debug)
      Serial.println(F("Turn off BT"));
      stateBT = 0;
      // turn on radio
      radioPower();
      lastMedia = FM;
    }
  }
  Serial.print(F("lastMedia: "));
  Serial.println(lastMedia);
  Serial.println();

  EEPROM.update(LASTMEDIA_ADDR, lastMedia);
}

void mediaPower() {

  if ((stateFM + stateMP3 + stateBT) == 0) { // if everything is off
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
    if (stateFM == 1) {
      // turn off radio
      radioPower();
    }
    else if (stateMP3 == 1) {
      // turn off mp3 module
      mp3Power();
    }
    else if (stateBT == 1) {
      // turn off BT
      Serial.println(F("power - Turn off BT"));
    }
    displayMode(DISABLED);
  }
}

void mp3Power() {
  if (stateMP3 == 1) {
    // turn off
    setGPIO(MP3_POWER, LOW);
    stateMP3 = 0;
  }
  else {
    // turn on
    setGPIO(MP3_POWER, HIGH);
    stateMP3 = 1;
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

void mp3VolUp() {
  setGPIO(MP3_VOLU, HIGH, 1);
}
void mp3VolDown() {
  setGPIO(MP3_VOLD, HIGH, 1);
}

void mp3VolDown(bool _start) {
  if (_start) {
    setGPIO(MP3_VOLD, HIGH);
  }
  else {
    setGPIO(MP3_VOLD, LOW);
  }
}

void mp3VolUp(bool _start) {
  if (_start) {
    setGPIO(MP3_VOLU, HIGH);
  }
  else {
    setGPIO(MP3_VOLU, LOW);
  }
}


void radioPower() {
  // Powers up the radio if its powered down, power it down if its powered up

  if (stateFM == OFF) {
    stateFM = ON;
    radio.init();
    radio.setBand(RADIO_BAND_FM);
    radio.setMono(true);
    setAudioOut(FM);
    radio.setMute(false);
    savedFreq = eepromRead16(FMFREQ_LADDR, FMFREQ_HADDR);
    radio.setFrequency(savedFreq);
    radio.setVolume(15); // TEMPORARY (0-15)
#ifdef _RADIO_DEBUG
    radioDebug();
#endif
    // add if alarm is not triggered for freq display
    if (Run_Mode != RM_ALARM_TRIG) {
      displayFrequency(radio.getFrequency());
    }
  }
  else {
    Serial.println(radio.getFrequency());
    savedFreq = radio.getFrequency(); // Save currently tuned frequency
    eepromUpdate16(FMFREQ_LADDR, FMFREQ_HADDR, savedFreq);
    radio.term(); // Turn off RDA
    stateFM = OFF;
  }

}

void radioSeek(bool direction) {
  if (direction == UP) {
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
  // change to switch case
  if (audio_out == BEEP) { // Y0
    setGPIO(0, LOW);
    setGPIO(1, LOW);
  }
  if (audio_out == FM) {  // Y1
    setGPIO(0, HIGH);
    setGPIO(1, LOW);
  }
  if (audio_out == MP3) { // Y2
    setGPIO(0, LOW);
    setGPIO(1, HIGH);
  }
  if (audio_out == BT) {  // Y3
    setGPIO(0, HIGH);
    setGPIO(1, HIGH);
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
