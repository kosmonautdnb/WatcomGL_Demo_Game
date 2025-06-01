#include "gl.h"
#include <math.h>
#include "textures.hpp"
#include "vector.hpp"

double levelScrollY = 0;
double levelScrollX = 0;

extern int XRES;
extern int YRES;

void clearFrame() {
  glClearColor(0.0,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

static const float randomLike(const int index) {
    int b = index ^ (index * 11) ^ (index / 17) ^ (index >> 16) ^ (index * 1877) ^ (index * 8332) ^ (index * 173);
    b = b ^ (b << 8) ^ (b * 23);
    b >>= 3;
    return (float)(b & 0xffff) / 0x10000;
}

static const float rand2d(int x, int y) {
  return randomLike((x*17)+randomLike(y*31)*0x4000);
}

static const float rand2dp(double x, double y) {
  const int xi = (int)floor(x);
  const int yi = (int)floor(y);
  double fx = x - (double)xi;
  double fy = y - (double)yi;
  fx*=fx;
  fy*=fy;
  const double p00 = rand2d(xi,yi);
  const double p10 = rand2d(xi+1,yi);
  const double p11 = rand2d(xi+1,yi+1);
  const double p01 = rand2d(xi,yi+1);
  const double top = (p10 - p00) * fx + p00;
  const double btm = (p11 - p01) * fx + p01;
  return (btm-top)*fy+top;
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
  const double sc = 0.01;
  const double scale = 0.5;
  const double dY = 22.0*scale;
  const double fY =  fmod(levelScrollY,dY);
  const double tY = levelScrollY - fY;
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
  int farX = 85+5;
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
      if (k.z > 120 && (k.y-tY>-1000)) k.z = 120;
      d *= perlin(k.x*sc*2,(k.y-tY-fY-levelScrollY*4)*sc*2)*0.5+0.5;
      glColor3f(d*r-subi,d*g-subi,d*b-subi);
      if (p<0.625&&p>0.575) {
        glColor3f(10,0,0);
      }
      if (p<0.9&&p>0.8) {
        glColor3f(0,10,10);
      }
      glPointSize(XRES*(particleSize*((x&1)*0.25+0.75)+randomLike(k.y-tY-fY+randomLike(k.x*13.0))*10.0)/640);
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
