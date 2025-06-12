#include "gl.h"
#include "text.hpp"
#include "truetype.hpp"
#include "util.hpp"
#include <stdlib.h>
#include "string.hpp"

unsigned int font1 = 0;
unsigned int font2 = 0;
extern int XRES;

void initText() {
  font1=stb_initfont(0,"aicon-vf.ttf",XRES*13/320); if (font1==0) ERROR("File: aicon-vf.ttf");
  font2=stb_initfont(1,"aicon-vf.ttf",40); if (font2==0) ERROR("File: aicon-vf.ttf");
}

void closeText() {
  stb_clearfont(0);
  stb_clearfont(1);
}

void drawText(int fontIndex, float xp, float yp, const String &text, uint32_t color, const float scale, float anchorX, float anchorY) {
  glDrawTextTTF(fontIndex,xp, yp, 0, scale, text.c_str(), color, anchorX, anchorY);
}
