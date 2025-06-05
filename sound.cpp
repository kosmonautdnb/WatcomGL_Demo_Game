#define MINIMP3_IMPLEMENTATION
#include "minimp3.hpp"
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

static uint8_t *currentMP3 = NULL;
static uint8_t *mp3Buffer = NULL;
static int mp3BufferBytes = 0;
static short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
static mp3dec_t mp3d;
static mp3dec_frame_info_t info;
static signed int *mixBuffer = NULL;
static bool mp3play = false;

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

static double sizeOfNewMp3Sample = 0;
static double posOfNewMp3Sample = 0;
static int lastUpdateP = 0;

void decodeOneMp3Chunk() {
  sizeOfNewMp3Sample = mp3dec_decode_frame(&mp3d, mp3Buffer, mp3BufferBytes, pcm, &info);
  posOfNewMp3Sample = 0;
  mp3Buffer += info.frame_bytes;
  mp3BufferBytes -= info.frame_bytes;
}

void auFrame() {
  if (mp3play) {
    const int l = sampleLength();
    int p = samplePosition();
    double sub = (double)info.hz/speakerFrequency;
    if (sub < 1) 
      sub = 1;
    while(lastUpdateP<p) {
      short *s = &pcm[(int)posOfNewMp3Sample];
      int *m = &mixBuffer[lastUpdateP%l];
      *m += *s;
      lastUpdateP++;
      sizeOfNewMp3Sample -= sub;
      posOfNewMp3Sample += sub;
      if (sizeOfNewMp3Sample<=0.0) {
        decodeOneMp3Chunk();
      }
    }
  }
}

void playMP3(const String &fileName) {
  mp3play = false;
  if (currentMP3 != NULL) 
    delete[] currentMP3;
  currentMP3 = NULL;
  FILE *in = fopen(fileName.c_str(),"rb");
  if (in == NULL) return;
  fseek(in,0,SEEK_END);
  int siz = ftell(in);
  fseek(in,0,SEEK_SET);
  currentMP3 = new uint8_t[siz];
  fread(currentMP3,1,siz,in);
  mp3Buffer = currentMP3;
  mp3BufferBytes = siz;
  lastUpdateP = samplePosition();
  decodeOneMp3Chunk();
  fclose(in);
  mp3dec_init(&mp3d);
  memset(&info,0,sizeof(mp3dec_frame_info_t));
  mp3play = true;
}

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
