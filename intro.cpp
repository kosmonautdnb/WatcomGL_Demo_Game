#include "gl.h"
#include "util.hpp"
#include "intro.hpp"
#include "image.hpp"
#include "string.hpp"
#include "meshes.hpp"
#include "sound.hpp"
#include <math.h>
#include "hashmap.hpp"
#include "keymtrix.hpp"
#include "text.hpp"
#include "textures.hpp"


#define FOV 70
#define ASPECT (320.0/200.0) //(16.0/9.0) sadly we can't use (16.0/9.0) here since the point sprites would be ellipsoid and not circular
#define MONITORASPECT (16.0/9.0) // we need this for perfectly round GL_POINT point sprites
#define NEARPLANE 0.1
#define FARPLANE 1000.0
#define CAMPOSZ (-120.0/8)
#define CAMPOSY (120.0/8)

unsigned int tex_birdOfLight;
unsigned int tex_birdOfLight_Glow;
unsigned int tex_stars;

double currentTime_intro = 0;
double timeDelta_intro = 0;
double lastTime_intro = 0;
double seconds_intro = 0;

#define STARCOUNT_INTRO 0x200
static Vector stars_intro[STARCOUNT_INTRO];

void hudStart();
void hudEnd();
void clearFrame();
extern Mesh *player;
extern HashMap<uint32_t,uint32_t> reColor;
void drawMesh(Mesh *mesh);

void loadStartScreen() {
  RGBAImage img;

  img = RGBAImage::fromFile("brdoflit.png");
  if (img.data==NULL) ERROR("Error reading: brdoflit.png");
  glGenTextures(1, &tex_birdOfLight);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  img = RGBAImage::fromFile("brdoflgl.png");
  if (img.data==NULL) ERROR("Error reading: brdoflgl.png");
  glGenTextures(1, &tex_birdOfLight_Glow);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight_Glow);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  img = RGBAImage::fromFile("stars.jpg");
  if (img.data==NULL) ERROR("Error reading: stars.jpg");
  glGenTextures(1, &tex_stars);
  glBindTexture(GL_TEXTURE_2D, tex_stars);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  int a = 80;
  int b = a / 2;
  for (int i = 0; i < STARCOUNT_INTRO; i++) {
    stars_intro[i] = Vector(randomLike(i*33+33)*a-b,randomLike(i*77+77)*a-b,randomLike(i*55+55)*a-b,randomLike(i*88+88)*3+0.5);
  }
}

void setCameraIntro() {
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

void displaySkyStars() {
  glEnable(GL_TEXTURE_2D);
  glDepthMask(GL_FALSE);
  glPushMatrix();
  glLoadIdentity();
  gluLookAt(0,0,CAMPOSZ,0,0,0,0,-1,0);
  glTranslatef(0,0,0);
  glRotatef(-seconds_intro*5.0,0,0,1);
  glRotatef(-seconds_intro*5.0,0,1,0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, glowTexture2);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_ONE);
  glBegin(GL_POINTS);
  glColor4f(0.1,0.3,0.3,1);
  for (int i = 0; i < STARCOUNT_INTRO; i++) {
    Vector &k = stars_intro[i];
    glPointSize(k.w);
    glVertex3f(k.x, k.y, k.z);
  }
  glEnd();
  glDisable(GL_BLEND);
  glPopMatrix();
  glDepthMask(GL_TRUE);
  glDisable(GL_TEXTURE_2D);
}

void displaySkyDome() {
  glDepthMask(GL_FALSE);
  glPushMatrix();
  glLoadIdentity();
  gluLookAt(0,0,CAMPOSZ,0,0,0,0,-1,0);
  glTranslatef(0,0,0);
  glRotatef(-seconds_intro*5.0,0,0,1);
  glRotatef(-seconds_intro*5.0,0,1,0);
  int tesselX = 64;
  int tesselY = 1;
  float radX = 15;
  float radY = 45;
  glEnable(GL_TEXTURE_2D);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_stars);
  glBegin(GL_QUADS);
  float k = 0.5;
  glColor4f(0,k*0.5*2,k*2,1);
  Vector p[4];
  Vector t[4];
  for (int y = 0; y < tesselY; y++) {  
    for (int x = 0; x < tesselX; x++) {
      for (int i = 0; i < 4; i++) {
        int rx = i & 1;
        int ry = i/2;
        float fx = (float)(x+rx)/tesselX;
        float fy = (float)(y+ry)/tesselY;
        p[i].x = sin(fx*2*PI)*radX;
        p[i].y = (ry-0.5)*radY*2;
        p[i].z = cos(fx*2*PI)*radX;
        t[i].x = fx;
        t[i].y = fy;
      }
      glTexCoord2f(t[1].x,t[1].y);glVertex3f(p[1].x,p[1].y,p[1].z);
      glTexCoord2f(t[0].x,t[0].y);glVertex3f(p[0].x,p[0].y,p[0].z);
      glTexCoord2f(t[2].x,t[2].y);glVertex3f(p[2].x,p[2].y,p[2].z);
      glTexCoord2f(t[3].x,t[3].y);glVertex3f(p[3].x,p[3].y,p[3].z);
    }
  }
  glEnd();
  glPopMatrix();
  glDepthMask(GL_TRUE);
  glDisable(GL_TEXTURE_2D);
}

void displayLogo() {
  double k = 0.425;
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight_Glow);
  glBegin(GL_QUADS);
  Vector p[4];
  Vector t[4];
  int tesselX = 4;
  int tesselY = 4;
  for (int y = 0; y < tesselY; y++) {  
    for (int x = 0; x < tesselX; x++) {
      for (int i = 0; i < 4; i++) {
        int rx = i & 1;
        int ry = i/2;
        float fx = (float)(x+rx)/tesselX;
        float fy = (float)(y+ry)/tesselY;
        p[i].x = fx*1280;
        p[i].y = fy*720*k;
        p[i].z = 0;
        t[i].x = fx;
        t[i].y = fy*k;
        t[i].z = -seconds_intro*4+sin(seconds_intro*0.3+t[i].x)*5+cos(seconds_intro*0.25+t[i].y)*4;
      }
      glColor4f(0.5,1,1,sin(t[1].z)*0.4+0.6);
      glTexCoord2f(t[1].x,t[1].y);glVertex3f(p[1].x,p[1].y,p[1].z);
      glColor4f(0.5,1,1,sin(t[0].z)*0.4+0.6);
      glTexCoord2f(t[0].x,t[0].y);glVertex3f(p[0].x,p[0].y,p[0].z);
      glColor4f(0.5,1,1,sin(t[2].z)*0.4+0.6);
      glTexCoord2f(t[2].x,t[2].y);glVertex3f(p[2].x,p[2].y,p[2].z);
      glColor4f(0.5,1,1,sin(t[3].z)*0.4+0.6);
      glTexCoord2f(t[3].x,t[3].y);glVertex3f(p[3].x,p[3].y,p[3].z);
    }
  }


  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight);
  glBegin(GL_QUADS);
  glColor4f(1,0.85,0.7,1);
  glTexCoord2f(1,0);glVertex3f(1280,0,0);
  glTexCoord2f(0,0);glVertex3f(0,0,0);
  glTexCoord2f(0,1*k);glVertex3f(0,720*k,0);
  glTexCoord2f(1,1*k);glVertex3f(1280,720*k,0);
  glEnd();
  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
}

void displayStartScreen() {
  lastTime_intro = auSeconds();
  setCameraIntro();
  do {
    currentTime_intro = auSeconds();
    timeDelta_intro = currentTime_intro-lastTime_intro;
    lastTime_intro = currentTime_intro;
    seconds_intro += timeDelta_intro;
    clearFrame();
    displaySkyDome();
    
    glPushMatrix();
    glTranslatef(0,0,0);
    setCameraIntro();
    glTranslatef(0,0,0);
    glRotatef(seconds_intro*25.0,0,0,1);
    glRotatef(seconds_intro*15.0,0,1,0);
    float b = 2.f;
    glScalef(b,b,b);
    drawMesh(player);
    glPopMatrix();
  
    displaySkyStars();
    
    hudStart();
    displayLogo();
    double o = 45;
    unsigned int col = 0xffffff80;
    int d = sin(seconds_intro*10)*0x5f+0xa0;
    unsigned int wh = d*0x01010101;
    int a = 0;
    bool c;
    c = a == 0;drawText(1280/2,720/2+o*0,(c?">":"")+String("Start Game")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
    c = a == 1;drawText(1280/2,720/2+o*1,(c?">":"")+String("Highscores")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
    c = a == 2;drawText(1280/2,720/2+o*2,(c?">":"")+String("Setup")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
    c = a == 3;drawText(1280/2,720/2+o*3,(c?">":"")+String("Credits")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
    hudEnd();
    glRefresh();
    if (glNextKey() != 0) break;
    if (isKeyPressed(SCANCODE_LSHIFT)) break;
    if (isKeyPressed(SCANCODE_RSHIFT)) break;
    if (isKeyPressed(SCANCODE_CTRL)) break;
    if (isKeyPressed(SCANCODE_ALT)) break;
  } while(1);
}

