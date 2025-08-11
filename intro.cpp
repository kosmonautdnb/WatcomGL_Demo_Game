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
#include "lvlscrn.hpp"
#include "config.hpp"
#include "sprite.hpp"
#include "openglfx.hpp"


#define FOV 70
#define ASPECT ((float)glFrameBufferWidth/(float)glFrameBufferHeight) //(16.0/9.0) sadly we can't use (16.0/9.0) here since the point sprites would be ellipsoid and not circular
#define MONITORASPECT (16.0/9.0) // we need this for perfectly round GL_POINT point sprites
#define NEARPLANE 0.1
#define FARPLANE 1000.0
#define CAMPOSZ (-120.0/8)
#define CAMPOSY (120.0/8)

unsigned int tex_birdOfLight;
unsigned int tex_birdOfLight_Glow;
unsigned int tex_stars;
unsigned int tex_trees2;

double currentTime_intro = 0;
double timeDelta_intro = 0;
double lastTime_intro = 0;
double seconds_intro = 0;

#define STARCOUNT_INTRO 0x200
static Vector stars_intro[STARCOUNT_INTRO];

void hudStart();
void hudEnd();
void clearFrame();
Mesh *playerf;
extern HashMap<uint32_t,uint32_t> reColor;
void drawMesh(Mesh *mesh);
extern int levelNr;

extern Sample *shotSound;
extern Sample *exploSound;

void screenShot();
static bool scanDown = false;
static bool scanUp = false;
static int a = 0;
static bool acceptKeyPressed = false;
static bool reclineKeyPressed = false;
static int screen = 0;
static bool muted = false;

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
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  img = RGBAImage::fromFile("trees2.png");
  if (img.data==NULL) ERROR("Error reading: trees2.png");
  glGenTextures(1, &tex_trees2);
  glBindTexture(GL_TEXTURE_2D, tex_trees2);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  img.free();

  int a = 80;
  int b = a / 2;
  for (int i = 0; i < STARCOUNT_INTRO; i++) {
    stars_intro[i] = Vector(randomLike(i*33+33)*a-b,randomLike(i*77+77)*a-b,randomLike(i*55+55)*a-b,randomLike(i*88+88)*3+0.5);
  }

  playerf = loadObject("playerf.obj");
  centerAndResizeObject(playerf,10.0);
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
        t[i].y = 1-fy;
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
  double k;
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);

  if (!USE_SPRITES) {  
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_trees2);
    glBegin(GL_QUADS);
    glColor4f(1,1,1,1);
    k = 0.0;
    glTexCoord2f(1,1-k);glVertex3f(1280,720*k,0);
    glTexCoord2f(0,1-k);glVertex3f(0,720*k,0);
    glTexCoord2f(0,0);glVertex3f(0,720,0);
    glTexCoord2f(1,0);glVertex3f(1280,720,0);
    glEnd();
  } else {
    drawSprite(Vector(1280/2,720/2,0),1280,720,tex_trees2,0xffffffff,SPRITEFLAG_NODEPTHCOMPARE|SPRITEFLAG_NODEPTHWRITE);
  }

  double r = -1+seconds_intro*1.0;
  r *= 200;
  if (r > 0) r = 0;
  r = 0;

  glBlendFunc(GL_SRC_ALPHA,GL_ONE);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight_Glow);
  glBegin(GL_QUADS);
  k = 0.425;
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
        p[i].y = fy*720*k+r;
        p[i].z = 0;
        t[i].x = fx;
        t[i].y = 1-fy*k;
        t[i].z = -seconds_intro*2+sin(seconds_intro*0.3+t[i].x)*5+cos(seconds_intro*0.25+t[i].y)*4;
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
  glEnd();   

  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex_birdOfLight);
  if (r != 0) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  glBegin(GL_QUADS);
  float k2;
  k2 = sin(seconds_intro*1.2+1)*0.4+0.6;glColor4f(k2,k2,k2*0.75,1);glTexCoord2f(1,1);glVertex3f(1280,r,0);
  k2 = sin(seconds_intro*0.7)*0.4+0.6;glColor4f(k2,k2,k2*0.75,1);glTexCoord2f(0,1);glVertex3f(0,r,0);
  k2 = sin(seconds_intro*1.7)*0.4+0.6;glColor4f(k2,k2,k2*0.75,1);glTexCoord2f(0,1-k);glVertex3f(0,720*k+r,0);
  k2 = sin(seconds_intro*1.1)*0.4+0.6;glColor4f(k2,k2,k2*0.75,1);glTexCoord2f(1,1-k);glVertex3f(1280,720*k+r,0);
  glEnd();

  glDisable(GL_BLEND);
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_DEPTH_TEST);
}

bool isAcceptKeyPressed() {
  if (isKeyPressed(SCANCODE_RSHIFT)) return true;
  if (isKeyPressed(SCANCODE_CTRL)) return true;
  if (isKeyPressed(SCANCODE_ALT)) return true;
  return false;
}

bool isReclineKeyPressed() {
  if (isKeyPressed(SCANCODE_ESCAPE)) return true;
  return false;
}

void frame_mainScreen() {
  hudStart();
  displayLogo();
  double o = 45;
  unsigned int col = 0xffffff80;
  int d = sin(seconds_intro*10)*0x5f+0xa0;
  unsigned int wh = d*0x01010101;
  bool c;
  if (isKeyPressed(SCANCODE_DOWN)) scanDown = true;
  if (isKeyPressed(SCANCODE_UP)) scanUp = true;
  if (!isKeyPressed(SCANCODE_DOWN)&&scanDown) {
    scanDown=false;
    a++;
    if (a>3) a = 3; else shotSound->play(Vector());
  }
  if (!isKeyPressed(SCANCODE_UP)&&scanUp) {
    scanUp=false;
    a--;
    if (a<0) a = 0; else shotSound->play(Vector());
  }
  c = a == 0;drawText(0,1280/2,720/2+o*0,(c?">":"")+String("Start Game")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
  c = a == 1;drawText(0,1280/2,720/2+o*1,(c?">":"")+String("Highscores")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
  c = a == 2;drawText(0,1280/2,720/2+o*2,(c?">":"")+String("Setup")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
  c = a == 3;drawText(0,1280/2,720/2+o*3,(c?">":"")+String("Credits")+(c?"<":""),c?wh:col,1.0,0.5,0.5);
  hudEnd();
  if (isAcceptKeyPressed()) acceptKeyPressed = true; else if (acceptKeyPressed) {exploSound->play(Vector());acceptKeyPressed = false; screen = a; if(a==0) screen = -1;}
  if (isReclineKeyPressed()) reclineKeyPressed = true; else if (reclineKeyPressed) {exploSound->play(Vector());reclineKeyPressed = false; screen = -1;}
}

void frame_setup() {
  hudStart();
  displayLogo();
  unsigned int col = 0xffffffff;
  drawText(0,1280/2,720/2,"PC-Speaker:"+String(muted?"OFF":"ON"),col,1.0,0.5,0.0);
  hudEnd();
  if (isAcceptKeyPressed()) acceptKeyPressed = true; else if (acceptKeyPressed) {exploSound->play(Vector());acceptKeyPressed = false; muted = !muted; auMuteAudio(muted);}
  if (isReclineKeyPressed()) reclineKeyPressed = true; else if (reclineKeyPressed) {exploSound->play(Vector());reclineKeyPressed = false; screen = 0;}
}

void frame_creditsScreen() {
  hudStart();
  displayLogo();
  float o = 40;
  float fo = o * 1-720/10;
  unsigned int col = 0xffff8000;
  drawText(0,1280/2,720/2+-fo/2+o*0,"Coding: Stefan Mader",col,1.0,0.5,0.0);
  //drawText(0,1280/2,720/2+-fo/2+o*1,"Musics: Ronny Doll",col,1.0,0.5,0.0);
  hudEnd();
  if (isAcceptKeyPressed()) acceptKeyPressed = true; else if (acceptKeyPressed) {exploSound->play(Vector());acceptKeyPressed = false; screen = 0;}
  if (isReclineKeyPressed()) reclineKeyPressed = true; else if (reclineKeyPressed) {exploSound->play(Vector());reclineKeyPressed = false; screen = 0;}
}

void frame_highScores() {
  hudStart();
  displayLogo();
  float o = 40;
  unsigned int col = 0xffffff00;
  for (int i = 0; i < 10; i++)
    drawText(0,1280/2,300+o*i,"ABC 00100000",col,1.0,0.5,0.0);
  hudEnd();
  if (isAcceptKeyPressed()) acceptKeyPressed = true; else if (acceptKeyPressed) {exploSound->play(Vector());acceptKeyPressed = false; screen = 0;}
  if (isReclineKeyPressed()) reclineKeyPressed = true; else if (reclineKeyPressed) {exploSound->play(Vector());reclineKeyPressed = false; screen = 0;}
}

void displayStartScreen() {
  lastTime_intro = auSeconds();
  setCameraIntro();
  screen = 0;
  a = 0;
  acceptKeyPressed = false;
  reclineKeyPressed = false;
  scanDown = false;
  scanUp = false;
  muted = true;
  auMuteAudio(muted);
  do {
    int currentKey = glNextKey();
    currentTime_intro = auSeconds();
    timeDelta_intro = currentTime_intro-lastTime_intro;
    lastTime_intro = currentTime_intro;
    seconds_intro += timeDelta_intro;
    clearFrame();
    displaySkyDome();
    
    glPushMatrix();
    glTranslatef(0,0,0);
    setCameraIntro();
    glTranslatef(0,(sin(seconds_intro*0.25)*0.5+0.5)*-20,0);
    glRotatef(seconds_intro*25.0,0,0,1);
    //glRotatef(seconds_intro*15.0,0,1,0);
    float b = 2.f;
    glScalef(b,b,b);
    reColor[0x4000c0ff] = 0x00;
    drawMesh(playerf);
    reColor.clear();
    glPopMatrix();
  
    displaySkyStars();
    //glFXAA();

    switch(screen) {
    case 0:frame_mainScreen();break;
    case 1:frame_highScores();break;
    case 2:frame_setup();break;
    default: frame_creditsScreen();break;
    }

    glRefresh();
    if (currentKey == GL_VK_F1) {
      screenShot();
    }
  } while(screen>=0);

  glDeleteTextures(1,&tex_birdOfLight);
  glDeleteTextures(1,&tex_birdOfLight_Glow);
  glDeleteTextures(1,&tex_stars);
  glDeleteTextures(1,&tex_trees2);

  levelNr = 1;
  displayLevelScreen();
}

