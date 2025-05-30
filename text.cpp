#include "gl.h"
#include "text.hpp"
#include "truetype.hpp"
#include <stdlib.h>

unsigned int font1 = 0;
extern int XRES;

void initText() {
  font1=stb_initfont("aicon-vf.ttf",XRES*13/320);
  if (font1==0) {glDone(); exit(0);}
}

void drawText(float xp, float yp, const char *text, uint32_t color, const float scale, float anchorX, float anchorY) {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,1280,0,720,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDrawTextTTF(false,xp, yp, 0, scale, text, color, anchorX, anchorY);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}
