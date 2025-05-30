#include "sound.hpp"
#include "speaker.hpp"
#include <stdlib.h>
#include <math.h>
#include "util.hpp"

float randomLike(const int i);

double currentSoundStartTime;
double currentSoundDuration;
double currentSoundPriority;

void Sample::play() {
  auPlaySample(this);
}

Sample *auLoadSample(int type, double priority) {
  Sample *ret=NULL;
  if (type == 0) {
    unsigned int len = speakerFrequency*0.25;
    unsigned short *sample = (unsigned short*)malloc(len*sizeof(unsigned short));
    for (int i = 0; i < len; i++) {
      double ri = (double)i / (speakerFrequency*3.0);
      ri = pow(ri,1.5);
      float f = sin(ri*2.0*PI*0.5);
      f = clamp(f,-1.f,1.f);
      sample[i]=f*32000+32768;
    }
    ret = new Sample();
    ret->priority = priority;
    ret->sample = convertSampleFromUInt16(sample,len);
    ret->sampleLength = len;
    free(sample);
  }
  if (type == 1) {
    unsigned int len = speakerFrequency*0.5;
    unsigned short *sample = (unsigned short*)malloc(len*sizeof(unsigned short));
    for (int i = 0; i < len; i++) {
      int k = i*3/len+1;
      sample[i]=(randomLike(i/k*k)*2-1)*32000+32768;
    }
    ret = new Sample();
    ret->priority = priority;
    ret->sample = convertSampleFromUInt16(sample,len);
    ret->sampleLength = len;
    free(sample);
  }
  return ret;
}

double auSeconds() {
  return speakerSeconds;
}

void auMuteAudio(bool mute) {
  muteSpeaker = mute;
}

void auPlaySample(Sample *sample) {
  bool playing = false;
  double sp = speakerSeconds*speakerFrequency;
  if (sp>currentSoundStartTime&&sp<currentSoundStartTime+currentSoundDuration) playing = true;
  if (sample->priority>=currentSoundPriority||(!playing)) {
    playSample(sample->sample,sample->sampleLength,false);
    currentSoundPriority = sample->priority;
    currentSoundDuration = sample->sampleLength;
    currentSoundStartTime = sp;
  }
}

void auSoundDriverOn() {
  enableSamplePlayback();
}

void auSoundDriverOff() {
  disableSamplePlayback();
}
