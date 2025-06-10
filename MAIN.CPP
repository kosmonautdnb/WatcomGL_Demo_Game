#include "gl.h"
#include "types.hpp"
#include "keymtrix.hpp"
#include "array.hpp"
#include "vector.hpp"
#include "object.hpp"
#include "pair.hpp"
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
#include "intro.hpp"
#include <dos.h>
#include <math.h>

int XRES = 320;
int YRES = 200;

Sample *shotSound = NULL;
Sample *enemyShotSound = NULL;
Sample *exploSound = NULL;
Sample *slideSound = NULL;
Sample *getReadySample = NULL;
Sample *powerUpSample = NULL;
Sample *weaponSample = NULL;

#define ALTW
//#define RECORDVIDEO
#define videoPath "r:\\vid%04d.png"

//#define BOSSTEST

bool performanceMode = false;
double levelLength = 3000;
double lastTime = 0;
double currentTime = 0;
double timeDelta = 0;
int currentKey = 0;
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
unsigned int currentFrame = 0;
double fps = 0;
int32_t score = 0;
int32_t lives = 4;
HashMap<uint32_t,uint32_t> reColor;
bool markDebug = false;
double levelScroll = 0;

enum {
  WEAPON_GREEN = 1,
};
int weaponColor = WEAPON_GREEN;
int weaponStrength = 1;

#include "hud.hpp"
#include "levelsrc.hpp"
#include "level1.hpp"

double playerAnimDuration = 1.0;

Array<GO*> getAllObjectsInCapsule(const Vector &capsuleStart, const Vector &capsuleEnd, double radius) {
  Array<GO*> ret;
  capsule[CAPSULE_PLAYERSHOT] = Capsule(capsuleStart, capsuleEnd, radius);
  for (int i = 0; i < gameObjects.size(); i++) {
    GO *o = gameObjects[i];
    if (!__RUNNING(o)) continue;
    GO_Collider_Enemy *v1 = dynamic_cast<GO_Collider_Enemy*>(o);
    if (v1 != NULL && v1->collideWithCapsule(CAPSULE_PLAYERSHOT)) {
        ret.push_back(o);
    }
    GO_Collider_LevelObject *v2 = dynamic_cast<GO_Collider_LevelObject*>(o);
    if (v2 != NULL && v2->collideWithCapsule(CAPSULE_PLAYERSHOT)) {
        ret.push_back(o);
    }
  }
  return ret;
}

void paintGreenLaser(const Vector &start,const Vector &end, bool hitObjects) {
  Vector end2 = end;
  bool hit = false;
  GO *nearestEnemy = NULL;
  {
    static unsigned int ka = 0; ka++;
    if ((ka & 15)==0) slideSound->play(start);
    Array<GO*> objects = getAllObjectsInCapsule(start,end,5);
    for (int i = 0; i < objects.size(); i++) {
      GO *o = objects[i];
      GO_Collider_Enemy *v1 = dynamic_cast<GO_Collider_Enemy*>(o);
      GO_Collider_LevelObject *v2 = dynamic_cast<GO_Collider_LevelObject*>(o);
      if (v1 != NULL || v2 != NULL) {
        Vector pos;
        double rad;
        if (v1 != NULL) {
          pos =  v1->colliderEnemyPosition;
          rad = v1->colliderEnemyRadius;
        }
        if (v2 != NULL) {
          pos = v2->colliderLevelObjectPosition;
          rad = v2->colliderLevelObjectRadius;
        }
        double y = pos.y;
        if (end2.y < y && y < start.y) {
          end2.y = y; 
          if(v1 != NULL)
            nearestEnemy = o;
          hit = true;
        }
      }
    }
  }
  
  if (nearestEnemy != NULL && hitObjects) {
    playerShotHitObject(end2,nearestEnemy);
    playerShotHitObject(end2,nearestEnemy);
  }
  Vector normal = normalize(end2-start);
  Vector tangent = normalize(cross(normal,Vector(0,0,1)));
  Vector dist = end2-start;
  int tesselY = 10;
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_ONE);
  glBegin(GL_QUADS);
  int xp[4] = {1,0,0,1};
  int yp[4] = {0,0,1,1};
  double radX = 5.0;
  static unsigned int ka = 0;
  for (int y = 0; y < tesselY; y++) {
    for (double x = -1; x < 1 ; x+=0.5) {
      for (int j = 0; j < 4; j++) {
        double rx = x + xp[j]*0.5;
        double ry = y + yp[j] + fabs(rx)*0.25;
        ry *= ((1-fabs(rx))+tesselY)/tesselY;
        Vector p = lerp(start,end2,ry/tesselY);
        p += tangent * radX * rx;
        p.x += 0.5; // OpenGL has an error here
        if (y == 0 && yp[j]==0) {
          if (randomLike(ka*33)<0.25)
            placeSmallSmoke(p+tangent*(randomLike(ka*45)*2-1)*5);
          ka++;
        }
        if (y == (tesselY-1) && yp[j]==1 && hit) {
          if (randomLike(ka*33)<0.25)
            placeEmitExplosion(p+tangent*(randomLike(ka*45)*2-1)*5);
          ka++;
        }
        double k = ry - currentTime*8 + fabs(rx)*1.0;
        k *= 1.0 - fabs(rx)*0.5;
        float r = 0.2*sin(k)+0.2;
        float g = 0.5*sin(k)+0.75;
        float b = 0.4*sin(k)+0.5;
        float s = 1.0-fabs(rx)*1.25;
        if (y == 0 && yp[j]==0) s *= 0.1;
        r *= s;
        g *= s;
        b *= s;
        glColor4f(r,g,b,1);
        glVertex3f(p.x,p.y,p.z);
      }
    }
  }
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);
  glEnd();
}

void shot() {
  if (playerAnimDuration>0) return;
  static double lastShotTime = currentTime;

  double distance = fabs(lastShotTime - currentTime);

  if (weaponStrength>=3)
    paintGreenLaser(playerPos+Vector(0,-10,0),playerPos+Vector(0,-150,0),distance>0.1);

  if (distance > 0.1) {
    shotSound->play(playerPos);
    static unsigned int rk = 0; rk++;
    lastShotTime = currentTime;
    for (int i = -1; i <= 1; i++) {
      Vector k = playerPos;  
      k.x += i*3;
      if (weaponStrength < 3)
        gameObjects.push_back(go_(new PlayerShot(k,Vector(0,-80,0))));
      if (weaponStrength > 1 && (rk&1)) {
        gameObjects.push_back(go_((new PlayerShot(k,Vector(-20,-80,0)))->vorlauf(i*0.025)));
        gameObjects.push_back(go_((new PlayerShot(k,Vector(+20,-80,0)))->vorlauf(-i*0.025)));
      }
    }
  }
}

void drawPlayer(Mesh *mesh, const Vector &pos) {
  glPushMatrix();
  if (playerAnimDuration<=0) {
    glTranslatef(pos.x,pos.y,pos.z);
    glRotatef(playerRoll,0,1,0);
    drawMesh(mesh);

    // white "hit"points
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
    double d = playerAnimDuration;
    playerAnimDuration -= timeDelta*0.5;
    if (playerAnimDuration<0.25&&d>=0.25)
      getReadySample->play(Vector());
  }
  glPopMatrix();
}

int main(int argc, const char *argv) {

  printf("Starting up..\n");
  const bool pureVGA = false;
  if (pureVGA) {
    glVGA();
  } else {
    if (!glVesa(XRES,YRES,32)) {
      printf("No Vesa 320x200x32 found.\n");
      sleep(1);
      if (!glVGA())
      {
        printf("No VGA device found.\n");
        exit(0);
      }
    }
  }
  printf("Starting up..\n");
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  installKeyboardHandler();
  printf("Keyboard installed..\n");
  auSoundDriverOn();
  printf("Sound driver installed..\n");
  //auMuteAudio(true);

  player = loadObject("player.obj");
  centerAndResizeObject(player,10.0);
  printf("Player loaded..\n");

  enemyShotSound = auLoadSample(0,0.25);
  slideSound = auLoadSample(2,1);
  shotSound = auLoadSample(0,5);
  exploSound = auLoadSample(1,10);
  getReadySample = auLoadSample(3,100);
  powerUpSample = auLoadSample(4,100);
  weaponSample = auLoadSample(5,100);

  printf("Sounds loaded..\n");

  loadTextures();
  initText();
  printf("Textures loaded..\n");
  loadStartScreen();
  printf("StartScreen loaded..\n");

  go_setupManagers();

  loadLevel1();
  printf("Level1 loaded..\n");
 
  buildLevel1();


  printf("Starting..\n");

  if (argc>1) 
    performanceMode = true;

#ifdef BOSSTEST
  levelScroll = levelLength-20;
#endif

  displayStartScreen();

  bool done = false;
  auSeconds() = 0;
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
    static unsigned int lastFPSFrame = currentFrame;
    if (fabs(lastFPSTime-currentTime)>1.0) {
      lastFPSTime = currentTime;
      fps = currentFrame-lastFPSFrame;
      lastFPSFrame = currentFrame;
    }

    if (levelScroll < levelLength)
      levelScroll += timeDelta * 10.0;
    else {
      levelScroll = levelLength;
    }

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
    static bool shiftPressed = false;
    if (isKeyPressed(SCANCODE_RSHIFT)||isKeyPressed(SCANCODE_TAB)) { // LSHIFT is not usable, cursor keys emit LSHIFT, too
      if (!shiftPressed) {
        placeExplosionRing(playerPos);
        playerBlue = !playerBlue;
        slideSound->play(playerPos);
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
    if (isKeyPressed(SCANCODE_CTRL)) {
      shot();
    }      
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
    static unsigned int currentFrame = 0;
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
