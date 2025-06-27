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

int levelNr = 1;
bool liveLost = false;
bool levelCompleted = false;
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
bool playerRed = false;
unsigned int currentFrame = 0;
double fps = 0;
int32_t score = 0;
int32_t lives = 4;
HashMap<uint32_t,uint32_t> reColor;
bool markDebug = false;
double levelScroll = 0;
double playerAnimDuration = 1.0;
double playerLifeLostAnimDuration = 0;

enum {
  WEAPON_GREEN = 1,
};
int weaponColor = WEAPON_GREEN;
int weaponStrength = 1;

#include "hud.hpp"
#include "levelsrc.hpp"
#include "level1.hpp"


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

class ShootExp : public GO, public GO_LifeTime, public GO_Paintable {
public:
  Vector dir;
  Vector pos;
  Vector t;
  ShootExp(const Vector &rp, const Vector &d) : GO(), GO_LifeTime(0.1), GO_Paintable() {
    dir = d;
    pos = rp;
    t = normalize(cross(normalize(dir),Vector(0,0,1)))*4;
    active = true;
  }
  virtual void paint(double dt) {
    double k = randomLike((index+seconds)*33)*2.0-1.0;
    Vector p = playerPos+Vector(0,-10.0,0)+pos;
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBegin(GL_TRIANGLES);
    glBlendFunc(GL_ONE,GL_ONE);
    float a = 0.25;
    glColor4f(a*1.5,a*1.25,a,1);
    glVertex3f(p.x,p.y,p.z);
    glVertex3f(p.x+dir.x,p.y+dir.y,p.z+dir.z);
    glVertex3f(p.x+dir.x*0.25+t.x*k,p.y+dir.y*0.25+t.y*k,p.z+dir.z*0.25+t.z*k);
    glEnd();
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
  }
};

void shot() {
  if (playerAnimDuration>0) return;
  static double lastShotTime = currentTime;

  double distance = fabs(lastShotTime - currentTime);

  if (weaponStrength>=3)
    paintGreenLaser(playerPos+Vector(0,-10,0),playerPos+Vector(0,-150,0),distance>0.1);

  if (distance > 0.1) {
    static unsigned int j = 0; 
    j++; gameObjects.push_back(go_(new ShootExp(Vector(0,0,0),Vector(randomLike(j*77)*4.0-2.0,-8,0))));
    if (weaponStrength > 1) {
      j++; gameObjects.push_back(go_(new ShootExp(Vector(-2,10,0),Vector(randomLike(j*77)*4.0-2.0,-8,0))));
      j++; gameObjects.push_back(go_(new ShootExp(Vector(+2,10,0),Vector(randomLike(j*77)*4.0-2.0,-8,0))));
    }

    shotSound->play(playerPos);
    static unsigned int rk = 0; rk++;
    lastShotTime = currentTime;
      for (int i = -1; i <= 1; i++) {
      Vector k = playerPos;  
      k.x += i*3;
      if (weaponStrength < 3)
        gameObjects.push_back(go_((new PlayerShot(k,Vector(0,-80,0)))->vorlauf(i==0?0.0125:0)));
      if (weaponStrength > 1 && (rk&1)) {
        gameObjects.push_back(go_((new PlayerShot(k,Vector(-20,-80,0)))->vorlauf(i*0.025)));
        gameObjects.push_back(go_((new PlayerShot(k,Vector(+20,-80,0)))->vorlauf(-i*0.025)));
      }
    }
  }
}

void drawPlayer(Mesh *mesh, const Vector &pos) {
  glPushMatrix();
  if (playerAnimDuration<=0&&playerLifeLostAnimDuration<=0) {
    glTranslatef(pos.x,pos.y,pos.z);
    glRotatef(playerRoll,0,1,0);
    drawMesh(mesh);

    // white "hit"points
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shotTexture[3]);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1f);
    glBegin(GL_POINTS);
    if (playerRed)
      glColor4f(1,0,0,1);
    else
      glColor4f(1,0,1,1);
    glPointSize(XRES*5/320);
    glVertex3f(0,0,0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    PlayerTurbineParticle::emit();

  } else {
    if (playerLifeLostAnimDuration>0) {
      Vector p = Vector(pos.x,pos.y,pos.z-pow(1-playerLifeLostAnimDuration,0.75)*100);
      glTranslatef(p.x,p.y,p.z);
      glRotatef((1-playerLifeLostAnimDuration)*45,1,0,0);
      glRotatef(pow((1-playerLifeLostAnimDuration),0.75)*360,0,1,0);
      drawMesh(mesh);
      placeExplosion(p);
    }
    else {
      glTranslatef(pos.x,pos.y,pos.z-pow(playerAnimDuration,0.75)*220);
      glRotatef(playerAnimDuration*45,1,0,0);
      glRotatef(pow(playerAnimDuration,0.75)*360,0,1,0);
      drawMesh(mesh);
      double d = playerAnimDuration;
      playerAnimDuration -= timeDelta*0.5;
      if (playerAnimDuration<0.25&&d>=0.25)
        getReadySample->play(Vector());
    }
  }
  glPopMatrix();
}

void screenShot() {
  RGBAImage m,m2;
  m.width = glFrameBufferWidth;
  m.height = glFrameBufferHeight;
  m.data = glFrameBuffer;
  m2 = m.getResized(1280, 720);
  for (int i = 0; i < 1280*720; i++) m2.data[i] |= 0xff000000;
  m2.savePNG("scrnshot.png");
  m2.free();
}

void reInitLevel() {
  loadLevel1();
  buildLevel1();
  auSeconds() = 0;
  lastTime = 0;
  playerRed = false;
  levelScroll = 0;
#ifdef BOSSTEST
  levelScroll = levelLength-20;
#endif
  playerAnimDuration = 1.0;
  playerLifeLostAnimDuration = 0;
  playerHits = 0;
  playerPosScreen = Vector();
}

int main(int argc, const char *argv) {

  printf("Starting up..\n");
  const bool pureVGA = false;
  printf("Starting up..\n");
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  installKeyboardHandler();
  printf("Keyboard installed..\n");
  auSoundDriverOn();
  printf("Sound driver installed..\n");
  auMuteAudio(true);

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

  if (argc>1) 
    performanceMode = true;

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

  displayStartScreen();

  bool done = false;
  reInitLevel();
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
        playerRed = !playerRed;
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
    switch(levelNr) {
      case 1:paintLevel1(); break;
      //case 2:paintLevel2(); break;
    }
    setCameraGame();
    drawPlayer(player,playerPos);
    if (isKeyPressed(SCANCODE_CTRL)) {
      shot();
    }      
    go_onFrame(timeDelta);
    drawHud();
    if (playerAnimDuration>0) drawHudGetReady(playerAnimDuration);
    glRefresh();
    if (liveLost) {
      liveLost = false;
      playerLifeLostAnimDuration=1.0;
    }
    if (levelCompleted) {
      sleep(5);
      done = true;
    }
    if (playerLifeLostAnimDuration>0) {
      playerLifeLostAnimDuration -= timeDelta;
      if (playerLifeLostAnimDuration<=0) {
        playerLifeLostAnimDuration=0;
        lives--;
        if (lives <= 0) done = true;
        reInitLevel();
      }
    }

    if (currentKey == GL_VK_F1) {
      screenShot();
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
