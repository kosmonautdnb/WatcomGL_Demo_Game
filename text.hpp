#ifndef __TEXT_HPP__
#define __TEXT_HPP__

#include "types.hpp"

void initText();
void drawText(float xp, float yp, const class String &text, uint32_t color=0xffffffff, const float scale = 1.f, float anchorX=0, float anchorY=0);

#endif //__TEXT_HPP__
