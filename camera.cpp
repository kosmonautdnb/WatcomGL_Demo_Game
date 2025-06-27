#include "gl.h"
#include "vector.hpp"
#include <math.h>


#define FOV 30
#define ASPECT ((float)glFrameBufferWidth/glFrameBufferHeight)//(320.0/200.0) //(16.0/9.0) sadly we can't use (16.0/9.0) here since the point sprites would be ellipsoid and not circular
#define MONITORASPECT (16.0/9.0) // we need this for perfectly round GL_POINT point sprites
#define NEARPLANE 0.1
#define FARPLANE 1000.0
#define CAMPOSZ -120.0
#define CAMPOSY 150.0

extern double levelScrollY;
extern double levelScrollX;

extern Vector playerPosScreen;

void setCameraBackground() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV,ASPECT,NEARPLANE,FARPLANE);
  glSetMonitorAspectRatio(MONITORASPECT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0,CAMPOSY,CAMPOSZ-playerPosScreen.y*0.1,0,0,0,0,-1,0);
  float c[4];
  c[0] = 1;
  c[1] = -1;
  c[2] = -1;
  c[3] = 0; // directional
  glLightfv(GL_LIGHT0,GL_POSITION,c);
  c[0]=c[1]=c[2]=0.25;
  glLightfv(GL_LIGHT0,GL_AMBIENT,c);
}

void setCameraGame() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV,ASPECT,NEARPLANE,FARPLANE);
  glSetMonitorAspectRatio(MONITORASPECT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0,-levelScrollY,CAMPOSZ*2,0,-levelScrollY,0,0,-1,0);
  float c[4];
  c[0] = 1;
  c[1] = -1;
  c[2] = -1;
  c[3] = 0; // directional
  glLightfv(GL_LIGHT0,GL_POSITION,c);
  c[0]=c[1]=c[2]=0.25;
  glLightfv(GL_LIGHT0,GL_AMBIENT,c);
}
