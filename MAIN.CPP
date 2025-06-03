#include "gl.h"
#include "types.hpp"
#include "keymtrix.hpp"
#include "array.hpp"
#include "vector.hpp"
#include "object.hpp"
#include "string.hpp"
#include "smplobjl.hpp"
#include "gameobj.hpp"
#include "gameomgr.hpp"
#include "textures.hpp"
#include "backgrnd.hpp"
#include "camera.hpp"
#include "sound.hpp"
#include "image.hpp"
#include "util.hpp"
#include "meshes.hpp"
#include "colision.hpp"
#include "text.hpp"
#include <math.h>

int XRES = 320;
int YRES = 200;

Sample *shotSound = NULL;
Sample *enemyShotSound = NULL;
Sample *exploSound = NULL;
Sample *slideSound = NULL;

//#define RECORDVIDEO
#define ALTW
#define videoPath "r:\\vid%04d.png"

double lastTime = 0;
double currentTime = 0;
double timeDelta = 0;
int currentKey = 0;
double scrollPosX=0;
double scrollPosY=0;
double walkingAngle=0;
Vector playerPosScreen;
Vector playerPos;
double playerSpeed = 50.0;
double playerRoll = 0;
double playerDestRoll = 0;
double playerRollSpeed = 100.0;
int playerHits=0;
int maxPlayerHits=20;
bool playerBlue = true;
int currentFrame = 0;
double fps = 0;
int32_t score = 0;
int32_t lives = 4;
HashMap<uint32_t,uint32_t> reColor;

#include "levelsrc.hpp"
#include "level1.hpp"
#include "hud.hpp"

double playerAnimDuration = 1.0;

void shot() {
  if (playerAnimDuration>0) return;
  static double lastShotTime = currentTime;
  double distance = fabs(lastShotTime - currentTime);
  if (distance > 0.1) {
    lastShotTime = currentTime;
    for (int i = -1; i <= 1; i++) {
      Vector k = playerPos;  
      k.x += i*3;
      gameObjects.push_back(go_(new PlayerShot(k,Vector(0,-80,0))));
      //gameObjects.push_back(go_(new PlayerShot(k,Vector(-20,-80,0)))->vorlauf(i*0.025));
      //gameObjects.push_back(go_(new PlayerShot(k,Vector(+20,-80,0)))->vorlauf(-i*0.025));
    }
  }
}

void drawPlayer(Mesh *mesh, const Vector &pos) {
  glPushMatrix();
  if (playerAnimDuration<=0) {
    glTranslatef(pos.x,pos.y,pos.z);
    glRotatef(playerRoll,0,1,0);
    drawMesh(mesh);
  
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shotTexture[3]);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1f);
    glBegin(GL_POINTS);
    glColor4f(1,1,1,1);
    glPointSize(XRES*5/320);
    glVertex3f(0,0,0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glEnable(GL_DEPTH_TEST);
    PlayerTurbineParticle::emit();
  } else {
    glTranslatef(pos.x,pos.y,pos.z-pow(playerAnimDuration,0.75)*220);
    glRotatef(playerAnimDuration*45,1,0,0);
    glRotatef(pow(playerAnimDuration,0.75)*360,0,1,0);
    drawMesh(mesh);
    playerAnimDuration -= timeDelta*0.5;
  }
  glPopMatrix();
}

double levelScroll = 0;
int main(int argc, const char *argv) {
  glVesa(XRES,YRES,32);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  installKeyboardHandler();
  auSoundDriverOn();
  auMuteAudio(true);

  player = loadObject("player.obj");
  centerAndResizeObject(player,10.0);
  enemyShotSound = auLoadSample(0,0.25);
  shotSound = auLoadSample(0,0.5);
  slideSound = auLoadSample(2,1);
  exploSound = auLoadSample(1,10);
  loadTextures();
  initText();

  go_setupManagers();

  loadLevel1();
  buildLevel1();

  bool done = false;
  do {
    currentKey = glNextKey();
    if (currentKey == GL_VK_ESCAPE) done = true;
    currentTime = auSeconds();
    timeDelta = currentTime-lastTime;
#ifdef ALTW
    if (isKeyPressed(SCANCODE_W) && isKeyPressed(SCANCODE_ALT)) {
      timeDelta *= 15;
    }
#endif
    lastTime = currentTime;
    currentFrame++;
    static double lastFPSTime = 0;
    static int lastFPSFrame = currentFrame;
    if (fabs(lastFPSTime-currentTime)>1.0) {
      lastFPSTime = currentTime;
      fps = currentFrame-lastFPSFrame;
      lastFPSFrame = currentFrame;
    }

    levelScroll += timeDelta * 10.0;
    levelScrollX = playerPosScreen.x;
    levelScrollY = levelScroll;

    clearFrame();
    static Vector lastPlayerPos = playerPos;
    capsule[CAPSULE_PLAYER] = Capsule(playerPos,lastPlayerPos,CAPSULE_PLAYER_RADIUS);
    lastPlayerPos = playerPos;

    if (playerDestRoll<playerRoll) playerRoll -= timeDelta*playerRollSpeed;
    if (playerDestRoll>playerRoll) playerRoll += timeDelta*playerRollSpeed;

    playerDestRoll = 0;
    if (isKeyPressed(SCANCODE_UP)) {
      playerPosScreen.y -= playerSpeed*timeDelta;
    }
    if (isKeyPressed(SCANCODE_DOWN)) {
      playerPosScreen.y += playerSpeed*timeDelta;
    }
    if (isKeyPressed(SCANCODE_LEFT)) {
      playerPosScreen.x -= playerSpeed*timeDelta;
      playerDestRoll = +45;
    }
    if (isKeyPressed(SCANCODE_RIGHT)) {
      playerPosScreen.x += playerSpeed*timeDelta;
      playerDestRoll = -45;
    }
    if (isKeyPressed(SCANCODE_CTRL)) {
      shot();
    }      
    static bool shiftPressed = false;
    if (isKeyPressed(SCANCODE_RSHIFT)) {
      if (!shiftPressed) {
        placeExplosionRing(playerPos);
        playerBlue = !playerBlue;
      }
      shiftPressed = true;
    } else {
      shiftPressed = false;
    }

    playerPosScreen.x = clamp(playerPosScreen.x,-110.0,110.0);
    playerPosScreen.y = clamp(playerPosScreen.y,-55.0,55.0);
    playerPos = playerPosScreen-Vector(0,levelScrollY,0);

    setCameraBackground();
    paintLevel();
    setCameraGame();
    drawPlayer(player,playerPos);
    go_onFrame(timeDelta);
    drawHud();
    if (playerAnimDuration>0) drawHudGetReady(playerAnimDuration);
    glRefresh();
    if (currentKey == GL_VK_F1) {
      RGBAImage m,m2;
      m.width = glFrameBufferWidth;
      m.height = glFrameBufferHeight;
      m.data = glFrameBuffer;
      m2 = m.getResized(1280, 720);
      m2.savePNG("scrnshot.png");
      m2.free();
    }
#ifdef RECORDVIDEO
  {
    static int currentFrame = 0;
    RGBAImage m;
    m.width = glFrameBufferWidth;
    m.height = glFrameBufferHeight;
    m.data = glFrameBuffer;
    char buffer[220];
    sprintf(buffer,videoPath,currentFrame);
    m.savePNG(buffer);
    currentFrame++;
  }
#endif
  } while(!done);
  auSoundDriverOff();
  uninstallKeyboardHandler();
  glDone();
  return 0;
}
