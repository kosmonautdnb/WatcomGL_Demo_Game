#include "sound.hpp"
#include "speaker.hpp"
#include <stdlib.h>
#include <math.h>
#include "util.hpp"
#include "vector.hpp"

float randomLike(const int i);

double currentSoundStartTime;
double currentSoundDuration;
double currentSoundPriority;
Sample *currentSoundSample;

bool isInScreen(const Vector &position);

void Sample::play(const Vector &pos) {
  if (isInScreen(pos))
    auPlaySample(this);
}

Sample *auLoadSample(int type, double priority) {
#define MAXV 0x80000
  Sample *ret=NULL;
  if (type == 0) {
    // shot
    unsigned int len = speakerFrequency*0.25;
    signed int *sample = (signed int*)malloc(len*sizeof(signed int));
    for (int i = 0; i < len; i++) {
      double ri = (double)i / (speakerFrequency*3.0);
      ri = pow(ri,2.5);
      double f = sin(ri*2.0*PI*0.5);
      f = clamp(f,-1.0,1.0);
      sample[i]=f*MAXV;
    }
    ret = new Sample();
    ret->priority = priority;
    ret->sample = sample;
    ret->sampleLength = len;
  }
  if (type == 1) {
    // explosion
    unsigned int len = speakerFrequency*0.5;
    signed int *sample = (signed int*)malloc(len*sizeof(signed int));
    for (int i = 0; i < len; i++) {
      int k = i*3/len+1;
      sample[i]=(randomLike(i/k*k)*2-1)*MAXV*0.1;
    }
    ret = new Sample();
    ret->priority = priority;
    ret->sample = sample;
    ret->sampleLength = len;
  }
  if (type == 2) {
    // slide
    unsigned int len = speakerFrequency*0.5;
    signed int *sample = (signed int*)malloc(len*sizeof(signed int));
    for (int i = 0; i < len; i++) {
      double ri = (double)i / (speakerFrequency*20.0);
      ri = pow(ri,0.25);
      float f = sin(ri*2.0*PI*0.5);
      f = clamp(f,-1.f,1.f);
      sample[i]=f*MAXV;
    }
    ret = new Sample();
    ret->priority = priority;
    ret->sample = sample;
    ret->sampleLength = len;
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
  currentSoundStartTime = auSeconds();
  currentSoundPriority = sample->priority;
  currentSoundDuration = sample->sampleLength;
  currentSoundSample = sample;
  playSample32BitSigned(sample->sample,sample->sampleLength,1.0);
}

signed int *mixBuffer = NULL;

void auSoundDriverOn() {
  if (mixBuffer != NULL) delete[] mixBuffer;
  double seconds = 10.0;  int mixBufferLength = (int)(speakerFrequency*seconds);  mixBuffer = new signed int[mixBufferLength*sizeof(signed int)];
  enable32BitSignedSamplePlayback(mixBuffer,mixBufferLength);
}

void auSoundDriverOff() {
  disableSamplePlayback();
  if (mixBuffer != NULL) delete[] mixBuffer;
  mixBuffer = NULL;
}
