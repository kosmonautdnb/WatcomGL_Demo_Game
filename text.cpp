#include "gl.h"
#include "text.hpp"
#include "truetype.hpp"
#include "util.hpp"
#include <stdlib.h>
#include "string.hpp"

unsigned int font1 = 0;
extern int XRES;

void initText() {
  font1=stb_initfont("aicon-vf.ttf",XRES*13/320);
  if (font1==0) ERROR("File: aicon-vf.ttf");
}

void drawText(float xp, float yp, const String &text, uint32_t color, const float scale, float anchorX, float anchorY) {
  glDrawTextTTF(false,xp, yp, 0, scale, text.c_str(), color, anchorX, anchorY);
}
