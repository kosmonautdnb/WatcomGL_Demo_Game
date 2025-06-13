#include "gl.h"
#include "util.hpp"
#include "string.hpp"
#include "image.hpp"
#include "keymtrix.hpp"
#include "sound.hpp"
#include "vector.hpp"
#include "meshes.hpp"
#include <math.h>
#include "hashmap.hpp"
#include "text.hpp"
#include "truetype.hpp"

#define FOV 70
#define ASPECT (320.0/200.0) //(16.0/9.0) sadly we can't use (16.0/9.0) here since the point sprites would be ellipsoid and not circular
#define MONITORASPECT (16.0/9.0) // we need this for perfectly round GL_POINT point sprites
#define NEARPLANE 0.1
#define FARPLANE 1000.0
#define CAMPOSZ (-60.0/8)
#define CAMPOSY (120.0/8)

class Particle {
public:
  Vector p;
  Vector d;
  double s;
  double l;
};

GLuint tex_face = 0;
GLuint tex_level = 0;

double seconds_levelScreen=0;

extern Sample *shotSound;
extern Sample *exploSound;
extern Mesh *playerf;
extern HashMap<uint32_t,uint32_t> reColor;
extern GLuint ftex;
extern int XRES;
extern int levelNr;
extern unsigned int glowTexture;
Array<Particle> particles;

void clearFrame();
void hudStart();
void hudEnd();
void drawMesh(Mesh *mesh);
void screenShot();
float randomLike(const unsigned int t);

static bool isAcceptKeyPressed() {
  if (isKeyPressed(SCANCODE_RSHIFT)) return true;
  if (isKeyPressed(SCANCODE_CTRL)) return true;
  if (isKeyPressed(SCANCODE_ALT)) return true;
  return false;
}

static bool isReclineKeyPressed() {
  if (isKeyPressed(SCANCODE_ESCAPE)) return true;
  return false;
}

static unsigned int loadTexture(const String &name) {
  RGBAImage img;
  unsigned int tex;

  img = RGBAImage::fromFile(name.c_str());
  if (img.data==NULL) ERROR("Error reading:" + name);
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();
  return tex;
}

void displayBackground() {
  glEnable(GL_TEXTURE_2D);
  glDepthMask(GL_FALSE);
  glActiveTexture(GL_TEXTURE0);

  float k = 0.375;
  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D,tex_face);
  glBegin(GL_QUADS);
  glColor4f(k,k,k,1);
  glTexCoord2f(1,0); glVertex3f(1280.0,0,0);
  glTexCoord2f(0,0); glVertex3f(0.0,0,0);
  glTexCoord2f(0,1); glVertex3f(0.0,720.0,0);
  glTexCoord2f(1,1); glVertex3f(1280.0,720.0,0);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void displayForeground() {
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glDepthMask(GL_FALSE);

  
  const char *levelNrStrs[] = {
    "zero",
    "one",
    "two",
    "three",
    "four",
    "five",
    "six",
    "seven",
    "eight",
    "nine",
    "ten"};
  const char *levelNrStr = levelNrStrs[levelNr];

  drawText(0,850,80,"Sys X-Alyph",0x00ffffff+(unsigned int)(sin(seconds_levelScreen*5)*0x1f+0xe0)*0x01000000,1.0,0.0,0.0);


  float k = 15;
  drawText(1,700,550+k,levelNrStr,0x80000000,1.0,0.0,0.0);

  //glEnable(GL_BLEND);
  //glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);
  //glBindTexture(GL_TEXTURE_2D,tex_level);
  //glBegin(GL_QUADS);
  //glColor4f(1,1,1,0.5);
  //glTexCoord2f(1,0); glVertex3f(1280.0,k,0);
  //glTexCoord2f(0,0); glVertex3f(0.0,k,0);
  //glTexCoord2f(0,1); glVertex3f(0.0,720.0+k,0);
  //glTexCoord2f(1,1); glVertex3f(1280.0,720.0+k,0);
  //glEnd();

  drawText(1,700,550,levelNrStr,0xffffffff,1.0,0.0,0.0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D,tex_level);
  glBegin(GL_QUADS);
  glColor4f(1,1,1,1);
  glTexCoord2f(1,0); glVertex3f(1280.0,0,0);
  glTexCoord2f(0,0); glVertex3f(0.0,0,0);
  glTexCoord2f(0,1); glVertex3f(0.0,720.0,0);
  glTexCoord2f(1,1); glVertex3f(1280.0,720.0,0);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void setCameraLevelScreen() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV,ASPECT,NEARPLANE,FARPLANE);
  glSetMonitorAspectRatio(MONITORASPECT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0,CAMPOSY,CAMPOSZ,0,0,0,0,-1,0);
  float c[4];
  c[0] = 1;
  c[1] = -1;
  c[2] = -1;
  c[3] = 0; // directional
  glLightfv(GL_LIGHT0,GL_POSITION,c);
  c[0]=c[1]=c[2]=0.25;
  glLightfv(GL_LIGHT0,GL_AMBIENT,c);
}

void hiPass() {
  for (int y = 0; y < glFrameBufferHeight-1; y++) {
    unsigned int *a = &glFrameBuffer[y*glFrameBufferWidth];
    for (int x = 0; x < glFrameBufferWidth-1; x++) {
      int r = (*a & 255)*3-(a[1] & 255) - (a[glFrameBufferWidth] & 255)- (a[glFrameBufferWidth+1] & 255);
      int g = ((*a>>8) & 255)*3-((a[1]>>8) & 255) - ((a[glFrameBufferWidth]>>8) & 255)- ((a[glFrameBufferWidth+1]>>8) & 255);
      int b = ((*a>>16) & 255)*3-((a[1]>>16) & 255) - ((a[glFrameBufferWidth]>>16) & 255)- ((a[glFrameBufferWidth+1]>>16) & 255);
      int c = ((r + g + b)/3*0.75);
      r = (*a & 255)+c;            
      g = ((*a>>8) & 255)+c;
      b = ((*a>>16) & 255)+c;
      if (r > 255) r = 255;
      if (g > 255) g = 255;
      if (b > 255) b = 255;
      if (r < 0) r = 0;
      if (g < 0) g = 0;
      if (b < 0) b = 0;
      *a = r|(g<<8)|(b<<16)|0xff000000;
      a++;
    }
  }
}

void greyScale() {
  for (int y = 0; y < glFrameBufferHeight; y++) {
    unsigned int *a = &glFrameBuffer[y*glFrameBufferWidth];
    for (int x = 0; x < glFrameBufferWidth; x++) {
      int r = (*a & 255);
      int g = ((*a>>8) & 255);
      int b = ((*a>>16) & 255);
      int c = ((r + g + b)/3);
      c *= 10;
      r = (r + c)/11;
      g = (g + c)/11;
      b = (b + c)/11;
      *a = r|(g<<8)|(b<<16)|0xff000000;
      a++;
    }
  }
}

void emitParticle(const Vector &p, const Vector &d, const double s) {
  Particle v;
  v.p = p;
  v.d = d;
  v.s = s;
  v.l = 1;
  if (particles.size()<200)
    particles.push_back(v);
}

void updateParticles(double dt) {
  for (int i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    p.p += dt * p.d;
    p.l -= dt;
    if (p.l <= 0) {particles.erase(i,1);i--;}
  }
}

void drawParticles() {
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_ONE);
  glDepthMask(GL_FALSE);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,glowTexture);
  glBegin(GL_POINTS);
  glColor4f(0.1,1,1,1);
  for (int i = 0; i < particles.size(); i++) {
    Particle &p = particles[i];
    glPointSize(XRES*p.s/320);
    glVertex3f(p.p.x,p.p.y,p.p.z);
  }
  glEnd();
  glDepthMask(GL_TRUE);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

void displayLevelScreen() {
  clearFrame();
  glRefresh();

  tex_face = loadTexture("face.png");
  tex_level = loadTexture("level.png");

  while(isAcceptKeyPressed());
  while(isReclineKeyPressed());

  auSeconds() = 0;
  seconds_levelScreen = auSeconds();
  double emitties = 0;
  do {
    int currentKey = glNextKey();
    clearFrame();
    double bef = seconds_levelScreen;
    seconds_levelScreen = auSeconds();

    updateParticles(seconds_levelScreen-bef);
    emitties += seconds_levelScreen-bef;
    double k = 0.005;
    int i = 0;
    while(emitties-k>=0) {
      static unsigned int ka = 0; ka++;
      emitParticle(Vector((int)(ka & 1)*2-1+randomLike(ka*2)*0.25-0.125,5,randomLike(ka*5)*0.25-0.1250),Vector(0,8,0)*(randomLike(ka*33)+0.1),randomLike(ka)*20+10);
      emitties-=k;
      i++;
      if (i > 20) break;
    }

    hudStart();
    displayBackground();
    hudEnd();

    setCameraLevelScreen();
    glTranslatef(0,0,0);
    glRotatef(seconds_levelScreen*25.0+180,0,0,1);
    float b = 2.f;
    glScalef(b,b,b);
    reColor[0x4000c0ff] = 0x00;
    drawMesh(playerf);
    reColor.clear();
    drawParticles();
    greyScale();

    hiPass();

    hudStart();
    displayForeground();
    hudEnd();
   
    glRefresh();
    if (currentKey == GL_VK_F1) {
      screenShot();
    }
    if (isAcceptKeyPressed()) break;
    if (isReclineKeyPressed()) break;
  } while(1);

  exploSound->play(Vector());

  while(isAcceptKeyPressed());
  while(isReclineKeyPressed());
  glNextKey();

  glDeleteTextures(1,&tex_face);
  glDeleteTextures(1,&tex_level);
}
