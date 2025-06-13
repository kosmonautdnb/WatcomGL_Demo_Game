#include "textures.hpp"
#include "gl.h"
#include <math.h>
#include "image.hpp"
#include "util.hpp"
#include "string.hpp"

unsigned int glowTexture = 0;
unsigned int glowTexture2 = 0;
unsigned int cloudTexture = 0;
unsigned int shotTexture[6] = {0};
unsigned int explosionTexture = 0;
unsigned int smokeTexture = 0;

void loadTextures() {
  int x,y;

  unsigned int *buffer = new unsigned int[128*128];

  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      int a = 255;
      if (d < 0) {d = 0;a=0;}
      d = pow(d,2);
      int r = d * 255.f;
      if (r <= 0) {r = 0; a=0;}
      if (r > 255) r = 255;
      buffer[x+y*32] = r | (r<<8) | (r<<16) | (a<<24);
    }
  }
  glGenTextures(1, &glowTexture);
  glBindTexture(GL_TEXTURE_2D, glowTexture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      int a = 255;
      if (d < 0) {d = 0;a=0;}
      d = pow(d,2);
      if ((x>=14&&x<=16)||(y>=14&&y<=16)) {
        int absx = abs(x-15);
        int absy = abs(y-15);
        int absxy = absx<absy?absx:absy;
        d *= 2.f-absxy; 
      }
      else d *= 0.5f;
      int r = d * 255.f;
      if (r <= 0) {r = 0; a=0;}
      if (r > 255) r = 255;
      buffer[x+y*32] = r | (r<<8) | (r<<16) | (a<<24);
    }
  }
  glGenTextures(1, &glowTexture2);
  glBindTexture(GL_TEXTURE_2D, glowTexture2);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // also linear
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


  RGBAImage img = RGBAImage::fromFile("big3.png");
  if (img.data==NULL) ERROR("Error reading: big3.png");
  glGenTextures(1, &cloudTexture);
  glBindTexture(GL_TEXTURE_2D, cloudTexture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  img = RGBAImage::fromFile("exp3.png");
  if (img.data==NULL) ERROR("Error reading: exp3.png(1)");
  glGenTextures(1, &explosionTexture);
  glBindTexture(GL_TEXTURE_2D, explosionTexture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  img = RGBAImage::fromFile("exp3.png");
  if (img.data==NULL) ERROR("Error reading: exp3.png(2)");
  glGenTextures(1, &smokeTexture);
  glBindTexture(GL_TEXTURE_2D, smokeTexture);
  int i; for (i = 0; i < img.width*img.height; i++) img.data[i] = ((unsigned int)(img.data[i]>>24)*(img.data[i] & 255)/255)*(unsigned int)(0x01010101);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  // dark
  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      float q = 1.f-sqrt((fx+0.25)*(fx+0.25)+(fy+0.25)*(fy+0.25));
      int r,g,b,a;
      r = 160;
      g = 90;
      b = 160;
      a = 255;
      if (d < 0) {a=0;}
      if (d > 0.5) {float s = 0.5;r*=s;g*=s;b*=s;}
      if (q > 0.7) {r=g=b=255;}
      buffer[x+y*32] = r | (g<<8) | (b<<16) | (a<<24);
    }
  }
  glGenTextures(1, &shotTexture[0]);
  glBindTexture(GL_TEXTURE_2D, shotTexture[0]);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // red
  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      float q = 1.f-sqrt((fx+0.25)*(fx+0.25)+(fy+0.25)*(fy+0.25));
      int r,g,b,a;
      r = 255;
      g = 128*0.5;
      b = 128*0.5;
      a = 255;
      if (d < 0) {a=0;}
      if (d < 0.2) {float s = 0.5;r*=s;g*=s;b*=s;}
      if (q > 0.7) {r=g=b=255;}
      buffer[x+y*32] = r | (g<<8) | (b<<16) | (a<<24);
    }
  }
  glGenTextures(1, &shotTexture[1]);
  glBindTexture(GL_TEXTURE_2D, shotTexture[1]);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // green
  for (y = 0; y < 64; y++) {
    float fy = y / 63.f * 2.f - 1.f;
    for (x = 0; x < 64; x++) {
      float fx = x / 63.f * 2.f - 1.f;
      float df = 1.f-sqrt(fx*fx+fy*fy);
      float dh = 1.f-sqrt(fx*fx+fy*fy)*3;
      df = clamp(df,0.f,1.f);
      dh = clamp(dh,0.f,1.f);
      int r,g,b,a;
      r = 128;
      g = 255;
      b = 128;
      a = 255;
      if (dh <= 0) {
        r = 0;
        g = 48*df*2;
        b = 32*df*2;
        a = 0;
      }
      if (df <= 0) {
        r = 0;
        g = 0;
        b = 0;
        a = 0;
      }
      buffer[x+y*64] = r | (g<<8) | (b<<16) | (a<<24);
    }
  }
  glGenTextures(1, &shotTexture[2]);
  glBindTexture(GL_TEXTURE_2D, shotTexture[2]);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,64,64,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // white
  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      float q = 1.f-sqrt((fx+0.25)*(fx+0.25)+(fy+0.25)*(fy+0.25));
      int r,g,b,a;
      r = 255;
      g = 255;
      b = 255;
      a = 255;
      if (d < 0) {a=0;}
      if (d < 0.2) {float s = 0.5;r*=s;g*=s;b*=s;}
      if (q > 0.7) {r=g=b=255;}
      buffer[x+y*32] = r | (g<<8) | (b<<16) | (a<<24);
    }
  }
  glGenTextures(1, &shotTexture[3]);
  glBindTexture(GL_TEXTURE_2D, shotTexture[3]);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


  // dark2
  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      float q = 1.f-sqrt((fx+0.25)*(fx+0.25)+(fy+0.25)*(fy+0.25));
      int r,g,b,a;
      int k = q * 300;     
      r = 160+k;
      g = 90+k;
      b = 160+k;
      a = 255*sin(d*PI);
      if (d < 0) a = 0;
      r = clamp(r,0,255);
      g = clamp(g,0,255);
      b = clamp(b,0,255);
      a = clamp(a,0,255);
      buffer[x+y*32] = r | (g<<8) | (b<<16) | (a<<24);
    }
  }
  glGenTextures(1, &shotTexture[4]);
  glBindTexture(GL_TEXTURE_2D, shotTexture[4]);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  // blue2
  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      float q = 1.f-sqrt((fx+0.25)*(fx+0.25)+(fy+0.25)*(fy+0.25));
      q=clamp(q,0.f,1.f);
      int r,g,b,a;
      int k = pow(q,4.0)*255;
      r = 160+k;
      g = 0+k;
      b = 0+k;
      a = 255*saturate(d*3.f);
      if (d < 0) a = 0;
      r = clamp(r,0,255);
      g = clamp(g,0,255);
      b = clamp(b,0,255);
      a = clamp(a,0,255);
      buffer[x+y*32] = r | (g<<8) | (b<<16) | (a<<24);
    }
  }
  glGenTextures(1, &shotTexture[5]);
  glBindTexture(GL_TEXTURE_2D, shotTexture[5]);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  delete[] buffer;
}

