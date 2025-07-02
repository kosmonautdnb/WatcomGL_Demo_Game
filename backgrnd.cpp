#include "gl.h"
#include <math.h>
#include "textures.hpp"
#include "vector.hpp"
#include "util.hpp"
#include "sprite.hpp"
#include "config.hpp"


double levelScrollY = 0;
double levelScrollX = 0;
double speedUpLevelScrollY = 0;

extern double currentTime;

extern int XRES;
extern int YRES;

void clearFrame() {
  glClearColor(0.0,0.0,0.0,1.0);
  glClearDepthf(1.0);
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

static __inline const unsigned short randomLikex(const unsigned short index) {
    int b = (int)index ^ (index * 11) ^ (index / 17) ^ (index * 1877) ^ (index * 8332) ^ (index * 173);
    b = b ^ (b << 8) ^ (b * 23);
    b >>= 3;
    return b & 0xffff;
}

static __inline const float randomLike2(const int index) {
    return (float)randomLikex(index)/0x10000;
}

static __inline const unsigned short rand2d(int x, int y) {
  return randomLikex((x*17)+randomLikex(y*31));
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

static __inline const float rand2dp2(double x, double y) {
  const int xi = (int)floor(x);
  const int yi = (int)floor(y);
  float fx = x - (double)xi;
  float fy = y - (double)yi;
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

static const float perlin2(double x, double y) {
  float r = 0;
  r += rand2dp2(x*1,y*1)/1.0;
  r += rand2dp2(x*2,y*2)/2.0;
  r += rand2dp2(x*4,y*4)/4.0;
  r += rand2dp(x*8,y*8)/8.0;
  r += rand2dp(x*16,y*16)/16.0;
  return r;
}

extern bool performanceMode;

#define MAXSTARS 500
double starX[MAXSTARS];
double starY[MAXSTARS];
double starSpeed[MAXSTARS];
bool starsInited = false;
                                                              
extern int glFrameBufferWidth;
extern int glFrameBufferHeight;
extern unsigned int *glFrameBuffer;


void paintLevel1Fast(double yPos) {
  static double lastYPos = yPos;
  if (!starsInited) {
    for (int i = 0; i < MAXSTARS; i++) {
      starX[i] = randomLike(i*17)*glFrameBufferWidth;
      starY[i] = randomLike(i*77+33)*glFrameBufferHeight;
      starSpeed[i] = randomLike(i*12+133)*2.0+1.0;
    }
    starsInited = true;
  }

  double speed = fabs(yPos-lastYPos)*3;
  double k = 1;
  if (speed<0.01) k = 0;

  for (int i = 0; i < MAXSTARS; i++) {
      int x = clamp((int)floor(starX[i]),0,glFrameBufferWidth-1);
      int r,g,b;
      for (int j = 0; j < 10; ++j) {
        int y = (int)floor(starY[i]+j*starSpeed[i]*0.25*k)-30;
        if (y < 0 || y >= glFrameBufferHeight) continue;
        int &rgba = glFrameBuffer[x+y*glFrameBufferWidth];
        r = rgba & 255;
        g = (rgba>>8) & 255;
        b = (rgba>>16) & 255;
        double k = (starSpeed[i]-0.75)/10.0;
        r += j * 80 * k;
        g += j * 120 * k;
        b += j * 255 * k;
        if (r>255) r = 255;
        if (g>255) g = 255;
        if (b>255) b = 255;
        rgba = r | (g<<8) | (b<<16) | 0xff000000;
      }

      starY[i] += starSpeed[i]*speed;
      starY[i] = fmod(starY[i],glFrameBufferHeight+30);
  }
  lastYPos = yPos;
}

void paintLevel2Fast(double yPos) {
  static double lastYPos = yPos;
  if (!starsInited) {
    for (int i = 0; i < MAXSTARS; i++) {
      starX[i] = randomLike(i*17)*glFrameBufferWidth;
      starY[i] = randomLike(i*77+33)*glFrameBufferHeight;
      starSpeed[i] = randomLike(i*12+133)*2.0+1.0;
    }
    starsInited = true;
  }

  double speed = fabs(yPos-lastYPos)*3;
  double k = 1;
  if (speed<0.01) k = 0;

  for (int i = 0; i < MAXSTARS; i++) {
      int x = clamp((int)floor(starX[i]),0,glFrameBufferWidth-1);
      int r,g,b;
      for (int j = 0; j < 10; ++j) {
        int y = (int)floor(starY[i]+j*starSpeed[i]*0.25*k)-30;
        if (y < 0 || y >= glFrameBufferHeight) continue;
        int &rgba = glFrameBuffer[x+y*glFrameBufferWidth];
        r = rgba & 255;
        g = (rgba>>8) & 255;
        b = (rgba>>16) & 255;
        double k = (starSpeed[i]-0.75)/10.0;
        r += j * 255 * k;
        g += j * 90 * k;
        b += j * 40 * k;
        if (r>255) r = 255;
        if (g>255) g = 255;
        if (b>255) b = 255;
        rgba = r | (g<<8) | (b<<16) | 0xff000000;
      }
      starY[i] += starSpeed[i]*speed;
      starY[i] = fmod(starY[i],glFrameBufferHeight+30);
  }
  lastYPos = yPos;
}

float _bg_col_r=0;
float _bg_col_g=0;
float _bg_col_b=0;
__inline glColor3f2(float r, float g, float b) {
  glColor3f(r,g,b);
  _bg_col_r = r;
  _bg_col_g = g;
  _bg_col_b = b;
}

void paintLevel1() {
  if (levelScrollY > 1000) speedUpLevelScrollY = (levelScrollY-1000)*2;
  const double levelScrollY2 = levelScrollY + speedUpLevelScrollY;
  if (performanceMode) {paintLevel1Fast(levelScrollY2); return;}
  const double sc = 0.01;
  const double scale = 0.5;
  const double dY = 22.0*scale;
  const double fY =  fmod(levelScrollY2,dY);
  const double tY = levelScrollY2 - fY;
  const double particleSize = 7.5;
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,2.f/255.f);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_ONE);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,glowTexture);
  glDepthMask(GL_FALSE);
  glDisable(GL_DEPTH_TEST);
  glBegin(GL_POINTS);
  glColor3f2(1,1,1);
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
      if (k.z > 120 && (y*dY-tY>-1000)) k.z = 120;
      d *= perlin(k.x*sc*2,(k.y-tY-fY-levelScrollY2*4)*sc*2)*0.5+0.5;
      glColor3f2(d*r-subi,d*g-subi,d*b-subi);
      if (p<0.625&&p>0.575) {
        glColor3f2(10,0,0);
      }
      if (p<0.9&&p>0.8) {
        glColor3f2(0,10,10);
      }
      bool bigStar = ((int)(k.y-tY-fY+k.x*33)&511)==0;
      unsigned int texture = bigStar?glowTexture2:glowTexture;
      glBindTexture(GL_TEXTURE_2D,texture);
      const float pointSize = (bigStar?3.0:1.0)*XRES*(particleSize*((x&1)*0.25+0.75)+randomLike2(k.y-tY-fY+randomLike2(k.x*13.0))*10.0)/640;
      glPointSize(pointSize);
      if (bigStar) glColor3f2(0.0,10,10);
      k.x -= levelScrollX*0.25;
      if (!USE_SPRITES || bigStar) {
        glVertex3f(k.x,k.y,k.z);
      } else {
        int r = (int)floor(_bg_col_r*255.f);
        int g = (int)floor(_bg_col_g*255.f);
        int b = (int)floor(_bg_col_b*255.f);
        r = clamp(r,0,255);
        g = clamp(g,0,255);
        b = clamp(b,0,255);
        drawSprite(Vector(k.x,k.y,k.z),pointSize,pointSize,texture,0xff000000|(r)|(g<<8)|(b<<16),SPRITEFLAG_NODEPTHWRITE|SPRITEFLAG_NODEPTHCOMPARE|SPRITEFLAG_ADDITIVE|SPRITEFLAG_BYSCREENSIZE);
      }
      lp = p;
    }
  }
  glEnd();
  glDepthMask(GL_TRUE);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_ALPHA_TEST);
}

