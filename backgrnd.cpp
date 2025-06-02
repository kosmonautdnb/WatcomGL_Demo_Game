#include "gl.h"
#include <math.h>
#include "textures.hpp"
#include "vector.hpp"

double levelScrollY = 0;
double levelScrollX = 0;
double speedUpLevelScrollY = 0;

extern int XRES;
extern int YRES;

void clearFrame() {
  glClearColor(0.0,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

static __inline const unsigned short randomLike(const unsigned short index) {
    int b = (int)index ^ (index * 11) ^ (index / 17) ^ (index * 1877) ^ (index * 8332) ^ (index * 173);
    b = b ^ (b << 8) ^ (b * 23);
    b >>= 3;
    return b & 0xffff;
}

static __inline const float randomLike2(const int index) {
    return (float)randomLike(index)/0x10000;
}

static __inline const unsigned short rand2d(int x, int y) {
  return randomLike((x*17)+randomLike(y*31));
}

static __inline const float rand2dp(double x, double y) {
  const int xi = (int)floor(x);
  const int yi = (int)floor(y);
  float fx = x - (double)xi;
  float fy = y - (double)yi;
  fx*=fx;
  fy*=fy;
  int fxi = fx * 0x400;
  int fyi = fy * 0x400;
  const unsigned short p00 = rand2d(xi,yi);
  const unsigned short p10 = rand2d(xi+1,yi);
  const unsigned short p11 = rand2d(xi+1,yi+1);
  const unsigned short p01 = rand2d(xi,yi+1);
  const unsigned short top = (((p10 - p00) * fxi)>>10) + p00;
  const unsigned short btm = (((p11 - p01) * fxi)>>10) + p01;
  return (float)((((btm-top)*fyi)>>10)+top)/0x10000;
}

static const float perlin(double x, double y) {
  y -= 1;
  float r = 0;
  r += rand2dp(x*1,y*1)/1.0;
  r += rand2dp(x*2,y*2)/2.0;
  r += rand2dp(x*4,y*4)/4.0;
  r += rand2dp(x*8,y*8)/4.0;
  //r += rand2dp(x*16,y*16)/16.0;
  return r;
}

void paintLevel() {
  if (levelScrollY > 1000) speedUpLevelScrollY = (levelScrollY-1000)*2;
  const double levelScrollY2 = levelScrollY + speedUpLevelScrollY;
  const double sc = 0.01;
  const double scale = 0.5;
  const double dY = 22.0*scale;
  const double fY =  fmod(levelScrollY2,dY);
  const double tY = levelScrollY2 - fY;
  const double particleSize = 7.5;
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_ONE);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,glowTexture);
  glDepthMask(GL_FALSE);
  glBegin(GL_POINTS);
  glColor3f(1,1,1);
  int yNear = 5+2;
  int yFar = -25-23-4;
  int nearX = 34+5+4;
  int farX = 85+5+5+5;
  for (int y = yFar; y < yNear; y++) {
    int xHere = (farX-nearX)*fabs(y-yNear)/fabs(yFar-yNear)+nearX;
    double lp = 0;
    for (int x = -xHere; x <= xHere; x++) {
      Vector k;
      k.x = x * 3.75;
      k.y = y * dY + sin(k.x*0.1*0.25+(y*dY-tY)*0.025)*dY;
      double p = perlin(k.x*sc,(k.y-tY)*sc);
      if ((p > 0.8)&&(x&1)) continue;
      k.y += fY;
      double d = 1.0+(lp-p)*10.0;
      if (d < 0) d  = 0;
      float r = 2-p*1.5;
      float g = 2-p;
      float b = p;
      float subi = 0.5;
      if (p>1.1) {
        r = 0.5;
        g = 0.75;
        b = 1;
      }
      k.z = p*120.0;
      if (k.z > 120 && (tY<1000)) k.z = 120;
      d *= perlin(k.x*sc*2,(k.y-tY-fY-levelScrollY2*4)*sc*2)*0.5+0.5;
      glColor3f(d*r-subi,d*g-subi,d*b-subi);
      if (p<0.625&&p>0.575) {
        glColor3f(10,0,0);
      }
      if (p<0.9&&p>0.8) {
        glColor3f(0,10,10);
      }
      glPointSize(XRES*(particleSize*((x&1)*0.25+0.75)+randomLike2(k.y-tY-fY+randomLike2(k.x*13.0))*10.0)/640);
      k.x -= levelScrollX*0.25;
      glVertex3f(k.x,k.y,k.z);
      lp = p;
    }
  }
  glEnd();
  glDepthMask(GL_TRUE);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}
