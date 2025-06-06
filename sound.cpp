#include "sound.hpp"
#include "speaker.hpp"
#include "util.hpp"
#include "vector.hpp"
#include <stdlib.h>
#include <math.h>

float randomLike(const int i);

static double currentSoundStartTime;
static double currentSoundDuration;
static double currentSoundPriority;
static Sample *currentSoundSample;

bool isInScreen(const Vector &position);

void Sample::play(const Vector &pos) {
  if (isInScreen(pos))
    auPlaySample(this);
}

Sample *auLoadSample(int type, double priority) {
  Sample *ret=NULL;
  if (type == 0) {
    // shot
    unsigned int len = speakerFrequency*0.25;
    unsigned short *sample = (unsigned short*)malloc(len*sizeof(unsigned short));
    for (int i = 0; i < len; i++) {
      double ri = (double)i / (speakerFrequency*3.0);
      ri = pow(ri,1.5);
      double f = sin(ri*2.0*PI*0.5);
      f = clamp(f,-1.0,1.0);
      sample[i]=f*32000+32768;
    }
    ret = new Sample();
    ret->priority = priority;
    ret->sample = convertSampleFromUInt16(sample,len);
    ret->sampleLength = len;
    free(sample);
  }
  if (type == 1) {
    // explosion
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
  if (type == 2) {
    // slide
    unsigned int len = speakerFrequency*0.5;
    unsigned short *sample = (unsigned short*)malloc(len*sizeof(unsigned short));
    for (int i = 0; i < len; i++) {
      double ri = (double)i / (speakerFrequency*20.0);
      ri = pow(ri,0.25);
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
  if (type == 3) {
    FILE *in = fopen("getready.raw","rb");
    fseek(in,0,SEEK_END);
    int siz = ftell(in);
    fseek(in,0,SEEK_SET);
    short *sample = (short*)malloc(siz);
    fread(sample,1,siz,in);
    double div = 48000.0/speakerFrequency;
    siz/=div;
    for (int i = 0; i < siz/sizeof(short);i++) {
      sample[i] = sample[(int)floor(i*div)]/1000;
    }
    ret = new Sample();
    ret->priority = priority;
    ret->sample = convertSampleFromSInt16(sample,siz/sizeof(short));
    ret->sampleLength = siz/sizeof(short);
    free(sample);
    fclose(in);
  }
  return ret;
}

volatile double &auSeconds() {
  return speakerSeconds;
}

void auMuteAudio(bool mute) {
  muteSpeaker = mute;
}

void auPlaySample(Sample *sample) {
  if (sample->priority < currentSoundPriority) {
    if (fabs(currentSoundStartTime-auSeconds())<currentSoundDuration)
      return;
  }
  currentSoundStartTime = auSeconds();
  currentSoundPriority = sample->priority;
  currentSoundDuration = sample->sampleLength/speakerFrequency;
  currentSoundSample = sample;
  playSample(sample->sample,sample->sampleLength,false);
}

void auFrame() {
}

void auSoundDriverOn() {
  enableSamplePlayback();
}

void auSoundDriverOff() {
  disableSamplePlayback();
}
