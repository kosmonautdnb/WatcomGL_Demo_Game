#include "gl.h"
#include "util.hpp"
#include "intro.hpp"
#include "image.hpp"
#include "string.hpp"

unsigned int tex_birdOfLight;
double introFade = 3.0;
extern double timeDelta;

void hudStart();
void hudEnd();

void loadStartScreen() {
  RGBAImage img;

  img = RGBAImage::fromFile("brdoflit.png");
  if (img.data==NULL) ERROR("Error reading: brdoflit.png");
  glGenTextures(1, &tex_birdOfLight);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();
}

void displayStartScreen() {
  if (introFade<=0) return;
  introFade -= timeDelta;
  hudStart();
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight);
  glBegin(GL_QUADS);
  glColor4f(1,1,1,introFade<1?introFade:1);
  glTexCoord2f(1,0);glVertex3f(1280,0,0);
  glTexCoord2f(0,0);glVertex3f(0,0,0);
  glTexCoord2f(0,1);glVertex3f(0,720,0);
  glTexCoord2f(1,1);glVertex3f(1280,720,0);
  hudEnd();
  glDisable(GL_BLEND);
}

