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
#include <math.h>

int XRES = 320;
int YRES = 200;

Sample *shotSound = NULL;
Sample *exploSound = NULL;


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
bool playerBlue = true;

#define CAPSULE_PLAYER_RADIUS 2

void drawMesh(Mesh *mesh) {
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
  float c[4];
  c[0]=1;
  c[1]=1;             
  c[2]=1;
  c[3]=1;
  glLightfv(GL_LIGHT0,GL_SPECULAR,c);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,c);
  c[0]=10;
  glMaterialfv(GL_FRONT_AND_BACK,GL_SHININESS,c);
  float mv[16];
  glGetFloatv(GL_MODELVIEW_MATRIX,mv);
  glBegin(GL_TRIANGLES);
  for (int i = 0; i < mesh->faces.size(); i++) {
    Face *f = &mesh->faces[i];
    Vector *p[3];
    p[0] = &mesh->vertices[f->v[0]];
    p[1] = &mesh->vertices[f->v[1]];
    p[2] = &mesh->vertices[f->v[2]];
    Vector *n[3];
    n[0] = &mesh->normals[f->n[0]];
    n[1] = &mesh->normals[f->n[1]];
    n[2] = &mesh->normals[f->n[2]];
    unsigned int col = f->color;
    if ((col>>24)==0x40) {
      glDisable(GL_LIGHTING);
      col = 0xffff0000;
      col |= (int)(randomLike(currentTime*200)*0xff)<<8;
    } else {
      glEnable(GL_LIGHTING);
    }
    if ((col>>24)==0x20) {
      col = playerBlue ? 0xff404040:0xffff00ff;
    }
    glColor4ubv((unsigned char*)&col);
    glNormal3f(n[0]->x,n[0]->y,n[0]->z); 
    glVertex3f(p[0]->x,p[0]->y,p[0]->z);
    glNormal3f(n[1]->x,n[1]->y,n[1]->z); 
    glVertex3f(p[1]->x,p[1]->y,p[1]->z);
    glNormal3f(n[2]->x,n[2]->y,n[2]->z); 
    glVertex3f(p[2]->x,p[2]->y,p[2]->z);
  }
  glEnd();
  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);
  glDisable(GL_COLOR_MATERIAL);
}

void drawPlayer(Mesh *mesh, const Vector &pos) {
  glPushMatrix();
  glTranslatef(pos.x,pos.y,pos.z);
  glRotatef(playerRoll,0,1,0);
  glRotatef(-90,0,1,0);
  glRotatef(-90,0,0,1);
  drawMesh(mesh);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, shotTexture[3]);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0.1f);
  glBegin(GL_POINTS);
  glColor4f(1,1,1,1);
  glPointSize(XRES*10/640);
  glVertex3f(0,0,0);
  glEnd();
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_ALPHA_TEST);
  glEnable(GL_DEPTH_TEST);

  glPopMatrix();
}

void drawEnemy(Mesh *mesh, const Vector &pos) {
  glPushMatrix();
  glTranslatef(pos.x,pos.y,pos.z);
  glRotatef(currentTime*80,sin(currentTime),cos(currentTime),0);
  drawMesh(mesh);
  glPopMatrix();
}

class Explosion : public GO, public GO_Position, public GO_LifeTime, public GO_Physical, public GO_Paintable {
public:
  double size;
  bool smoke;
  bool black;
  Explosion(const Vector &p, const Vector &v, double size, double lifeTime, bool smoke, bool black) : GO(), GO_Position(p), GO_LifeTime(lifeTime), GO_Physical(v), GO_Paintable() {
    this->size = size;
    this->smoke = smoke;
    this->black = black;
  }
  virtual void paint() {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    int xp[4]={1,-1,-1,1};
    int yp[4]={-1,-1,1,1};
    double b = 0.000;
    double tx[4]={1-b,b,b,1-b};
    double ty[4]={b,b,1-b,1-b};
    if (smoke) {
      glBindTexture(GL_TEXTURE_2D, smokeTexture);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      glColor4f(1,1,1,1);
      if (black) {
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        glColor4f(1,1,1,0.5);
      }
    } else {
      glBindTexture(GL_TEXTURE_2D, explosionTexture);
      glBlendFunc(GL_SRC_ALPHA,index & 1 ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1,1,1,1);
    }
    int framesW = 4;
    int framesH = 4;
    int currentFrame = 15-floor(lifeTime * 16 / initialLifeTime);
    double currentFrameX = currentFrame % framesW;
    double currentFrameY = currentFrame / framesW;
    for (int i2 = 0; i2 < 4; i2++) {
      tx[i2]/=framesW;
      ty[i2]/=framesH;
      tx[i2]+=currentFrameX/framesW;
      ty[i2]+=currentFrameY/framesH;
    }
    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);
    glRotatef(lifeTime*120,0,0,1);
    glBegin(GL_QUADS);
    for (int i = 0; i < 4; i++) {
      Vector p = Vector(xp[i],yp[i])*size;
      glTexCoord2f(tx[i],ty[i]);
      glVertex3f(p.x,p.y,p.z);
    }
    glEnd();
    glPopMatrix();
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
};

void placeSmallExplosion(const Vector &p) {
  gameObjects.push_back(go_(new Explosion(p,Vector(),2,1.0,true,false)));
  shotSound->play();
}

void placeExplosion(const Vector &p) {
  static int k;
  int i;
  for (i = 0; i < 10; i++) {
    float ri = i * PI * 2 / 10;
    gameObjects.push_back(go_(new Explosion(p,Vector(sin(ri)*120.0,cos(ri)*120.0,0),5,0.25,true,false)));
  }
  for (i = 0; i < 10; i++) {
    if (i & 1)
      gameObjects.push_back(go_(new Explosion(p,Vector((randomLike(k*11)-0.5)*80.0,(randomLike(k*543)-0.5)*80.0,0),5+randomLike(k*33)*15.0,0.25+randomLike(k*373)*0.5,false,false)));
    else
      gameObjects.push_back(go_(new Explosion(p,Vector((randomLike(k*11)-0.5)*80.0,(randomLike(k*543)-0.5)*80.0,0),5+randomLike(k*33)*15.0,0.25+randomLike(k*373)*0.5,true,true)));
    k++;
  }
  for (i = 0; i < 3; i++) {
    gameObjects.push_back(go_(new Explosion(p,Vector((randomLike(k*11)-0.5)*80.0,(randomLike(k*543)-0.5)*80.0,0),5+randomLike(k*33)*5.0,0.25+randomLike(k*373)*0.5,true,false)));
    k++;
  }
  exploSound->play();
}

void playerShotHitObject(const Vector &shotPos, GO *o) {
  if (dynamic_cast<GO_HitPoints*>(o) != NULL) {
    dynamic_cast<GO_HitPoints*>(o)->hitPoints--;
    if (dynamic_cast<GO_HitPoints*>(o)->hitPoints>0) {
      placeSmallExplosion(shotPos);
      return;
    }
  }
  o->deleteIt = true;
  if (dynamic_cast<GO_Position*>(o) != NULL) {
    placeExplosion(dynamic_cast<GO_Position*>(o)->position);
  }
}

class PlayerShot : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance {
public:
  Vector lastPosition;
  bool fresh;
  PlayerShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180) {
    position = p;
    velocity = v;
    shotSound->play();
    fresh = true;
  }
  virtual void paint() {
    if (!fresh) {
      capsule[CAPSULE_PLAYERSHOT] = Capsule(position, lastPosition, CAPSULE_PLAYER_RADIUS);
      for (int i = 0; i < gameObjects.size(); i++) {
        GO *o = go_(gameObjects[i]);
        if (o->deleteIt) continue;
        GO_Collider_Enemy *v0 = dynamic_cast<GO_Collider_Enemy*>(o);
        if (v0 != NULL) {
          capsule[CAPSULE_COLLIDER] = Capsule(v0->colliderEnemyPosition, v0->lastColliderEnemyPosition, v0->colliderEnemyRadius);
          if (collide(CAPSULE_COLLIDER,CAPSULE_PLAYERSHOT)) {
            deleteIt = true;
            playerShotHitObject(position,o);
          }
        }
      }
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shotTexture[2]);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1f);
    glBegin(GL_POINTS);
    glColor4f(1,1,1,1);
    glPointSize(XRES*6/640);
    glVertex3f(position.x,position.y,position.z);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    lastPosition = position;
    fresh = false;
  }
  PlayerShot *vorlauf(double dt) {
    position += velocity * dt;
    return this;
  }
};

class EnemyShot : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance, public GO_LifeTime {
public:
  Vector initialPosition;
  bool blue;
  Vector lastPosition;
  bool fresh;
  EnemyShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180), GO_LifeTime(100) {
    initialPosition = p;
    position = p;
    velocity = v;
    blue = false;
    fresh = true;
  }
  virtual void paint() {
    capsule[CAPSULE_COLLIDER] = Capsule(position, lastPosition, CAPSULE_PLAYER_RADIUS);
    if ((fresh == false) && collide(CAPSULE_COLLIDER,CAPSULE_PLAYER) && blue==playerBlue) {
      deleteIt = true;
      placeExplosion(position);
    }
    Vector position2 = position;
    Vector z = position2 - initialPosition;
    float ang = atan2(z.x,z.y) + lifeTime*0;
    float dist = length(z);
    position2 = Vector(sin(ang)*dist,cos(ang)*dist) + initialPosition;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shotTexture[blue?1:0]);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1f);
    glBegin(GL_POINTS);
    glColor4f(1,1,1,1);
    glPointSize(XRES*10/640);
    glVertex3f(position2.x,position2.y,position2.z);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    lastPosition = position;
    fresh = false;
  }
};

class SimpleCircleShotEmitter : public GO, public GO_Position, public GO_FrequencyCallback, public GO_Callback, public GO_Collider_Enemy, public GO_Paintable, public GO_HitPoints {
public:
  int sectors;
  double shotSpeed;
  SimpleCircleShotEmitter(const Vector &p, double frequency, double shotSpeed, int sectors) : GO(), GO_Position(p), GO_FrequencyCallback(frequency), GO_Callback(), GO_Collider_Enemy(20), GO_Paintable(), GO_HitPoints(64) {
    this->sectors = sectors;
    this->shotSpeed = shotSpeed;
  }
  virtual void frequent(int iteration) {
    float speed = 10.0;
    for (int i = 0; i < sectors; i++) {
      float a = i * 2 * PI / sectors + iteration;
      Vector dir(sin(a)*shotSpeed,cos(a)*shotSpeed,0);
      GO *enemyShot = go_(new EnemyShot(position,dir));
      if (((iteration % 12)==0)&&(i&1))
        dynamic_cast<EnemyShot*>(enemyShot)->blue=true;
      gameObjects.push_back(enemyShot);
    }
    if ((iteration % 12)==0)
      shotSound->play();
  }
  virtual void callback(double dt) {
  }
  virtual void paint() {
    drawEnemy(enemy,position);
  }
};

void shot() {
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

void playerHit() {
  placeExplosion(playerPos);
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
  enemy = loadObject("impostor.obj");
  centerAndResizeObject(player,10.0);
  centerAndResizeObject(enemy,10.0);
  shotSound = auLoadSample(0,0.5);
  exploSound = auLoadSample(1,10);
  loadTextures();

  go_setupManagers();
   
  gameObjects.push_back(go_(new SimpleCircleShotEmitter(Vector(-100,-200,0),0.2,32,20)));
  gameObjects.push_back(go_(new SimpleCircleShotEmitter(Vector(+100,-200,0),0.2,32,40)));
  gameObjects.push_back(go_(new SimpleCircleShotEmitter(Vector(0,-400,0),0.8,32,80)));

  bool done = false;
  do {
    currentTime = auSeconds();
    timeDelta = currentTime-lastTime;
    lastTime = currentTime;

    levelScroll += timeDelta * 10.0;
    levelScrollX = playerPosScreen.x;
    levelScrollY = levelScroll;

    clearFrame();
    setCameraBackground();
    paintLevel();
    setCameraGame();
    static Vector lastPlayerPos = playerPos;
    capsule[CAPSULE_PLAYER] = Capsule(playerPos,lastPlayerPos,CAPSULE_PLAYER_RADIUS);
    lastPlayerPos = playerPos;
    drawPlayer(player,playerPos);
    currentKey = glNextKey();
    if (currentKey == GL_VK_ESCAPE) done = true;


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
    if (isKeyPressed(SCANCODE_SPACE)) {
      shot();
    }      
    if (isKeyPressed(SCANCODE_1)) {
      playerBlue = false;
    }      
    if (isKeyPressed(SCANCODE_2)) {
      playerBlue = true;
    }      

    playerPosScreen.x = clamp(playerPosScreen.x,-110.0,110.0);
    playerPosScreen.y = clamp(playerPosScreen.y,-55.0,55.0);
    playerPos = playerPosScreen-Vector(0,levelScrollY,0);

    go_onFrame(timeDelta);
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
  } while(!done);
  auSoundDriverOff();
  uninstallKeyboardHandler();
  glDone();
  return 0;
}
