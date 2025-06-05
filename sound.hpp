#ifndef __SOUND_HPP__
#define __SOUND_HPP__

#include "string.hpp"

class Sample {
public:
  signed int *sample;
  int sampleLength;
  double priority;
  void play(const class Vector &pos);
};

Sample *auLoadSample(int type, double priority);
void auPlaySample(Sample *sample);
double auSeconds();
void auMuteAudio(bool mute);

void auFrame();
void playMP3(const String &fileName);

void auSoundDriverOn();
void auSoundDriverOff();

#endif //__SOUND_HPP__
