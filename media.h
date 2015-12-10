#ifndef _MEDIA_H
#define _MEDIA_H

#define _RADIO_DEBUG

#define UP 1
#define DOWN 0
#define ON 1
#define OFF 0

// GPIOs (Temporary!)
#define MP3_POWER 10 //  on GPIO[10] (MCP23017 PORTA Bit 7, See GPIO.h)
#define MP3_VOLU 9
#define MP3_VOLD 8
#define MP3_NEXT 7
#define MP3_PREV 6
#define S1_4051 0
#define S2_4051 1
#define AUDIO_EN 3

#define FMFREQ_LADDR 42
#define FMFREQ_HADDR 43
#define LASTMEDIA_ADDR 44

extern bool stateFM;
extern bool stateMP3;
extern bool stateBT;
extern uint8_t Media_Mode;

void setAudioOut(uint8_t media_mode);
void mediaPower();
void mediaChange();
void setVolume(bool _direction);
void radioPower();
void radioSeek(bool _direction);
void mp3Power();
void mp3Next();
void mp3Prev();
void mp3FF(bool _start);
void mp3Rew(bool _start);
// Temporary: 
void mp3VolDown();
void mp3VolUp();
void mp3VolDown(bool _start);
void mp3VolUp(bool _start);

#ifdef _RADIO_DEBUG
void radioDebug();
#endif

#endif
