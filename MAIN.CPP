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
int32_t score = 0;
int32_t lives = 4;

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
      col = (!playerBlue) ? 0xff0000ff:0xffff00ff;
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
  virtual void paint(double dt) {
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
      glColor4f(1,1,1,0.5);
      if (black) {
        glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
        glColor4f(1,1,1,0.25);
      }
    } else {
      glBindTexture(GL_TEXTURE_2D, explosionTexture);
      glBlendFunc(GL_SRC_ALPHA,index & 1 ? GL_ONE : GL_ONE_MINUS_SRC_ALPHA);
      glColor4f(1,1,1,0.75);
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

class ExplosionRing : public GO, public GO_Position, public GO_LifeTime, public GO_Paintable {
public:
  bool _slow;
  ExplosionRing(const Vector &p) : GO(), GO_Position(p), GO_LifeTime(0.5), GO_Paintable() {
   _slow = false;
  }
  virtual void paint(double dt) {
    double l = (double)(initialLifeTime-lifeTime)/initialLifeTime;
    double q = 5+l * (8.0+(_slow?20:0));
    double c = (1-l)*(_slow?0.25:1);
    double qr = (5.0-l*5)*(_slow?1.25:1);
    double xa[4]={1,0,0,1};
    double ya[4]={0,0,1,1};
    int tessel = 24;
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glBegin(GL_QUADS);
    glColor4f(c,c,c,1);
    for (int r = 0; r < tessel; r++) {
      for (int i = 0; i < 4; i++) {
        glVertex3f(position.x+sin((double)(xa[i]+r)/tessel*PI*2)*(q+ya[i]*qr),position.y+cos((double)(xa[i]+r)/tessel*PI*2)*(q+ya[i]*qr),0);
      }
    }
    glEnd();
    glDisable(GL_BLEND);
  }
  ExplosionRing *slow() {_slow=true;lifeTime*=2;initialLifeTime*=2;return this;}
};

class ExplosionFlare : public GO, public GO_Position, public GO_LifeTime, public GO_Paintable {
public:
  ExplosionFlare(const Vector &p) : GO(), GO_Position(p), GO_LifeTime(0.5), GO_Paintable() {
  }
  virtual void paint(double dt) {
    double l = (double)(initialLifeTime-lifeTime)/initialLifeTime;
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    float r = sin(l*PI)*20.0;
    float k = 1.0;
    for (int i = 0; i < 5; i++) {
      float a1 = randomLike(i*33+index*77)*2*PI+seconds*(randomLike(i)+1)*3;
      float a2 = a1+PI*0.15;
      glBegin(GL_TRIANGLES);
      glColor3f(0,0,0);
      glVertex3f(position.x,position.y,position.z);   
      glColor3f((1-l)*k,(1-l)*k*0.5,(1-l)*k*0.25);
      glVertex3f(position.x+sin(a1)*r,position.y+cos(a1)*r,position.z);      
      glColor3f((1-l)*0.5*k,(1-l)*0.5*k*0.5,(1-l)*0.5*k*0.25);
      glVertex3f(position.x+sin(a2)*r,position.y+cos(a2)*r,position.z);      
      glEnd();
    }
    glDisable(GL_BLEND);
  }
};

class PlayerTurbineParticle : public GO, public GO_LifeTime, public GO_Paintable {
public:
  Vector displace;
  PlayerTurbineParticle(const Vector &p) : displace(p), GO(), GO_LifeTime(0.5), GO_Paintable() {
    activated = true;
    static int k = 0; k++;
    lifeTime+=randomLike(k*111)*0.25;
  }
  virtual void paint(double dt) {
    double l = (double)(initialLifeTime-lifeTime)/initialLifeTime;
    Vector position = playerPos + displace + Vector(0,10,0);
    position.y += l * 20;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, glowTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glPointSize(XRES*4/320);
    glBegin(GL_POINTS);
    double k = 1 - l;
    glColor3f(0.25*k,0.85*k,1.0*k);
    glVertex3f(position.x, position.y, position.z);
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
  }
  static void emit() {
    static double t = 0;
    t += timeDelta;
    if (fabs(t)>0.0125) {
      for (int i = 0; i < 4; i++) {
        static int k=0; k++;
        t = 0;
        double r = 1.0;
        double x = 0;
        double y = 0;
        gameObjects.push_back(go_(new PlayerTurbineParticle(Vector(x+(randomLike(k*33)*2-1)*r,y+(randomLike(k*33)*2-1)*r,0))));
      }
    }
  }
};

void placeSmallSmoke(const Vector &p) {
  gameObjects.push_back(go_(new Explosion(p,Vector(),2,1.0,true,false)));
  shotSound->play(p);
}

void placeEmitExplosion(const Vector &p) {
  gameObjects.push_back(go_(new Explosion(p,Vector(),3,0.75,false,false)));
}

void placeTailExplosion(const Vector &p) {
  static int k = 0; k++;
  double a = k*2*PI/20;
  double sp = 10.0;
  gameObjects.push_back(go_(new Explosion(p,Vector(sin(a)*sp,cos(a)*sp,0),randomLike(k*33)*2.5+1.5,1.5,false,false)));
}

void placeExplosionRing(const Vector &p) {
  gameObjects.push_back(go_(new ExplosionRing(p)));
}

void placeExplosion(const Vector &p) {
  static int k;
  int i;
  gameObjects.push_back(go_(new Explosion(p,Vector(),25,0.125,false,false)));
  gameObjects.push_back(go_(new ExplosionRing(p)));
  gameObjects.push_back(go_((new ExplosionRing(p))->slow()));
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
  gameObjects.push_back(go_(new ExplosionFlare(p)));
  exploSound->play(p);
}

void playerShotHitObject(const Vector &shotPos, GO *o) {
  if (dynamic_cast<GO_ScoreHit*>(o)!=NULL) {
    score += dynamic_cast<GO_ScoreHit*>(o)->scoreHit;
  }

  if (dynamic_cast<GO_HitPoints*>(o) != NULL) {
    dynamic_cast<GO_HitPoints*>(o)->hitPoints--;
    if (dynamic_cast<GO_HitPoints*>(o)->hitPoints>0) {
      placeSmallSmoke(shotPos);
      return;
    }
  }

  if (dynamic_cast<GO_ScoreDestructed*>(o)!=NULL) {
    score += dynamic_cast<GO_ScoreDestructed*>(o)->scoreDestructed;
  }

  if (dynamic_cast<GO_HitPoints*>(o) != NULL)
    o->destruct();
}

bool isInScreen(const Vector &position) {
  const double y = position.y + levelScrollY;
  if (y > -65.0 && y < 65.0) return true;
  return false;
}

class PlayerShot : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance {
public:
  Vector lastPosition;
  bool fresh;
  PlayerShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180) {
    position = p;
    velocity = v;
    shotSound->play(p);
    fresh = true;
  }
  virtual void paint(double dt) {
    if (isInScreen(position)) {
      if (!fresh) {
        capsule[CAPSULE_PLAYERSHOT] = Capsule(position, lastPosition, CAPSULE_PLAYER_RADIUS);
        for (int i = 0; i < gameObjects.size(); i++) {
          GO *o = go_(gameObjects[i]);
          if (o->deleteIt) continue;
          GO_Collider_Enemy *v0 = dynamic_cast<GO_Collider_Enemy*>(o);
          if (v0 != NULL && (!v0->colliderEnemyFresh)) {
            capsule[CAPSULE_COLLIDER] = Capsule(v0->colliderEnemyPosition, v0->lastColliderEnemyPosition, v0->colliderEnemyRadius);
            if (collide(CAPSULE_COLLIDER,CAPSULE_PLAYERSHOT)) {
              destruct();
              playerShotHitObject(position,o);
            }
          }
          GO_Collider_LevelObject *v1 = dynamic_cast<GO_Collider_LevelObject*>(o);
          if (v1 != NULL && (!v1->colliderLevelObjectFresh)) {
            capsule[CAPSULE_COLLIDER] = Capsule(v1->colliderLevelObjectPosition, v1->lastColliderLevelObjectPosition, v1->colliderLevelObjectRadius);
            if (collide(CAPSULE_COLLIDER,CAPSULE_PLAYERSHOT)) {
              destruct();
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
      glPointSize(XRES*3/320);
      glVertex3f(position.x,position.y,position.z);
      glEnd();
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_ALPHA_TEST);
    }
    lastPosition = position;
    fresh = false;
  }
  PlayerShot *vorlauf(double dt) {
    position += velocity * dt;
    return this;
  }
};

void playerHit(bool explosion);

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
  virtual void paint(double dt) {
    capsule[CAPSULE_COLLIDER] = Capsule(position, lastPosition, CAPSULE_PLAYER_RADIUS);
    if ((blue==playerBlue) && (fresh == false) && collide(CAPSULE_COLLIDER,CAPSULE_PLAYER)) {
      destruct();
      placeExplosion(position);
      playerHit(false);
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
    glPointSize(XRES*5/320);
    glVertex3f(position2.x,position2.y,position2.z);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    lastPosition = position;
    fresh = false;
  }
  virtual void destruct() {
    placeEmitExplosion(position);
    GO::destruct();
  }
};

class Enemy1 : public GO, public GO_Position, public GO_FrequencyCallback, public GO_FrequencyCallback2, public GO_Collider_Enemy, public GO_Paintable, public GO_HitPoints, public GO_ScoreHit, public GO_ScoreDestructed {
public:
  Enemy1(const Vector &p) : GO(), GO_Position(p), GO_FrequencyCallback(0.2), GO_FrequencyCallback2(0.15), GO_Collider_Enemy(30), GO_Paintable(), GO_HitPoints(64), GO_ScoreHit(1), GO_ScoreDestructed(200) {
  }
  virtual void frequent(int iteration) {
    double k2 = fmod(seconds,9);
    int step = (int)floor(seconds/9);
    if (k2 < 1)
      return;

    int k = iteration / 6 + 1;
    if (k > 5) k = 5;
    for (int j = 0; j < k; j++) {
      double t = (seconds*((step&1)?90:-90)+j*360/5)*PI/180;
      double sp = 20;
      placeEmitExplosion(position+Vector(cos(t)*10,sin(t)*10,0));
      GO *enemyShot = go_(new EnemyShot(position,Vector(cos(t)*sp,sin(t)*sp,0)));
      dynamic_cast<EnemyShot*>(enemyShot)->blue=step&1;
      gameObjects.push_back(enemyShot);
      enemyShotSound->play(position);
    }
  }
  virtual void frequent2(int iteration) {
    double k2 = fmod(seconds,9);
    int step = (int)floor(seconds/9);
    if (k2 >= 1)
      return;
    for (int i = 0; i < 10; i++) {
      double a = i * 2 * PI / 10.0;
      placeEmitExplosion(position+Vector(sin(a)*9,cos(a)*9,0));
    }
  }
  double lk2;
  virtual void paint(double dt) {
    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);
    float k = sin(seconds*3)*0.1+0.9;
    double k2 = fmod(seconds,9);
    int step = (int)floor(seconds/9);
    glRotatef(seconds*((step&1)?90:-90),0,0,1);
    glScalef(k,k,k);
    if (k2 < 1) {
      glRotatef(k2*360,0,1,0);
      if (lk2 >= 1) slideSound->play(position);
      if (seconds > 5)
        position.x += dt*20;
    }
    lk2 = k2;
    drawMesh(enemy1);
    glPopMatrix();
  }

  virtual void destruct() {
    placeExplosion(position);
    GO::destruct();
  }
};

class Enemy2 : public GO, public GO_Position, public GO_FrequencyCallback, public GO_FrequencyCallback2, public GO_Collider_Enemy, public GO_Paintable, public GO_HitPoints, public GO_ScoreHit, public GO_ScoreDestructed {
public:
  bool blue;
  bool _circularShots;
  Enemy2(const Vector &p, bool blue) : GO(), GO_Position(p), GO_FrequencyCallback(0.35), GO_FrequencyCallback2(0.025), GO_Collider_Enemy(30), GO_Paintable(), GO_HitPoints(64), GO_ScoreHit(1), GO_ScoreDestructed(200) {
    this->blue = blue;
    _circularShots = false;
  }
  virtual void frequent(int iteration) {
    double k = (seconds/2.0)-1;
    k = 1.0-pow(fabs(k),2.0);
    if (k<0) return;
    for (int i = 0; i <= 10; i++) {
      double t = ((i-5)*(_circularShots?90:25)/5+90-atan2(playerPos.x-position.x,playerPos.y-position.y)*180/PI)*PI/180.f;
      double t2 = ((i-5)*(_circularShots?90:25)/5+90)*PI/180.f;
      double sp = 10+(5-fabs(i-5))*4;
      placeEmitExplosion(position+Vector(cos(t2)*sp*0.5,sin(t2)*sp*0.5,0));
      GO *enemyShot = go_(new EnemyShot(position,Vector(cos(t)*sp,sin(t)*sp,0)));
      dynamic_cast<EnemyShot*>(enemyShot)->blue=blue;
      gameObjects.push_back(enemyShot);
      enemyShotSound->play(position);
    }
  }
  virtual void frequent2(int iteration) {
    double k = (seconds/2.0)-1;
    k = 1.0-pow(fabs(k),2.0);
    if (k>0) return;
    placeTailExplosion(position+Vector(randomLike(iteration*33)*8-4,randomLike(iteration*77)*8-4,0));
  }
  double lk;
  virtual void paint(double dt) {
    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);
    double k = (seconds/2.0)-1;
    k = 1.0-pow(fabs(k),2.0);
    position.y += k;
    position.x -= position.x * dt * 0.1;
    glRotatef(180.f,0,1,0);
    glRotatef(90.f-(k>-1?k:-1)*90.f+180,1,0,0);
    if (k<0) glRotatef(k*180,0,1,0);
    if (k<0&&lk>=0) exploSound->play(position);
    drawMesh(enemy2);
    glPopMatrix();
    lk = k;
  }
  Enemy2 *circularShots() {
    _circularShots = true;
    return this;
  }
  virtual void destruct() {
    placeExplosion(position);
    GO::destruct();
  }
};

class Enemy3 : public GO, public GO_Position, public GO_FrequencyCallback, public GO_Collider_Enemy, public GO_Paintable, public GO_HitPoints, public GO_ScoreHit, public GO_ScoreDestructed {
public:
  double shotSpeed;
  int sectors;
  Enemy3(const Vector &p) : GO(), GO_Position(p), GO_FrequencyCallback(0.35), GO_Collider_Enemy(30), GO_Paintable(), GO_HitPoints(64), GO_ScoreHit(1), GO_ScoreDestructed(200) {
    shotSpeed = 10;
    sectors = 20;
  }
  virtual void frequent(int iteration) {
    float speed = 10.0;
    for (int i = 0; i < sectors; i++) {
      float a = i * 2 * PI / sectors + iteration;
      Vector dir(sin(a)*shotSpeed,cos(a)*shotSpeed,0);
      placeEmitExplosion(position+Vector(cos(a)*10,sin(a)*10,0));
      GO *enemyShot = go_(new EnemyShot(position,dir));
      if (((iteration % 12)==0)&&(i&1))
        dynamic_cast<EnemyShot*>(enemyShot)->blue=true;
      gameObjects.push_back(enemyShot);
    }
    enemyShotSound->play(position);
  }
  virtual void paint(double dt) {
    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);
    glRotatef(seconds*90,sin(seconds*3),cos(seconds*3),0);
    drawMesh(enemy3);
    glPopMatrix();
  }
  virtual void destruct() {
    placeExplosion(position);
    GO::destruct();
  }
};

class LevelObject : public GO, public GO_Position, public GO_Paintable, public GO_Rotation, public GO_Collider_LevelObject {
public:
  Mesh *mesh;
  LevelObject(const Vector &p, const Vector &r, Mesh *mesh) : GO(), GO_Position(p), GO_Paintable(), GO_Rotation(r.w,r.xyz()), GO_Collider_LevelObject(20) {
    this->mesh = mesh;
  }
  virtual void paint(double dt) {
    glPushMatrix();
    position.y += dt * 30.0;
    glTranslatef(position.x,position.y,position.z);
    glRotatef(angle,axis.x,axis.y,axis.z);
    drawMesh(mesh);
    glPopMatrix();
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

void playerHit(bool explosion) {
  playerHits++;
  if (explosion)
    placeExplosion(playerPos);
}

void drawEnergyBar(double xp, double yp, double w, double h) {
  double shorten=(double)playerHits/maxPlayerHits;
  float r2 = shorten;
  float g2 = 1-saturate(shorten+0.5);
  float b2 = 0;
  float r1 = shorten;
  float g1 = 1*(1-shorten);
  float b1 = 0;  
  double aw = w;
  w *= 1-shorten;
  xp += aw-w;
  glBegin(GL_QUADS);
  glColor4f(1,1,1,1);
  glColor3f(r2,g2,b2);
  glVertex3f(xp+w,yp,0);
  glColor3f(r1,g1,b1);
  glVertex3f(xp,yp,0);
  glVertex3f(xp,yp+h,0);
  glColor3f(r2,g2,b2);
  glVertex3f(xp+w,yp+h,0);
  glEnd();
}

void drawHud() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,1280,720,0,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  uint32_t color = 0xffffffc0;
  char scoreBuffer[16];
  sprintf(scoreBuffer,"%08d",score);
  drawText(0,0,scoreBuffer,color);
  sprintf(scoreBuffer,"%02d",lives);
  drawText(1280,0,scoreBuffer,color,1,1,0);

  drawEnergyBar(1280-250,5,150,20);

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void drawPlayer(Mesh *mesh, const Vector &pos) {
  glPushMatrix();
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

  glPopMatrix();
  PlayerTurbineParticle::emit();
}

double levelScroll = 0;
int main(int argc, const char *argv) {
  glVesa(XRES,YRES,32);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  installKeyboardHandler();
  auSoundDriverOn();
  //auMuteAudio(true);

  player = loadObject("player.obj");
  enemy = loadObject("impostor.obj");
  enemy1 = loadObject("enemy1.obj");
  enemy2 = loadObject("enemy2.obj");
  enemy3 = loadObject("enemy3.obj");
  object1 = loadObject("object1.obj");
  centerAndResizeObject(player,10.0);
  centerAndResizeObject(enemy,10.0);
  centerAndResizeObject(enemy1,10.0);
  centerAndResizeObject(enemy2,15.0);
  centerAndResizeObject(enemy3,10.0);
  centerAndResizeObject(object1,30.0);
  enemyShotSound = auLoadSample(0,0.25);
  shotSound = auLoadSample(0,0.5);
  slideSound = auLoadSample(2,1);
  exploSound = auLoadSample(1,10);
  loadTextures();
  initText();

  go_setupManagers();
   
  int i,j;
  for (i = 0; i < 10; i++) {
    gameObjects.push_back(go_(new Enemy1(Vector(randomLike(i*33)*150-75,-100-i*20,0))));
  }
  for (i = 0; i < 5; i++) {
    bool blue = i & 1;
    double x = randomLike(i*77)*50+50;
    gameObjects.push_back(go_(new Enemy2(Vector(x,-450-i*50,0),blue)));
    gameObjects.push_back(go_(new Enemy2(Vector(-x,-450-i*50,0),blue)));
    if (i>2)
      gameObjects.push_back(go_(new Enemy2(Vector(0,-450-i*50-25,0),blue)));
  }
  for (i = 0; i < 3; i++) {
    gameObjects.push_back(go_(new Enemy1(Vector(-75,-450-5*50-i*80,0))));
    gameObjects.push_back(go_(new Enemy1(Vector(75,-450-5*50-i*80,0))));
  }

  for (i = 0; i < 10; i++) {
    if (i & 1) {
      gameObjects.push_back(go_(new Enemy3(Vector(randomLike(i*33)*150-75,-1100-i*80,0))));
    } else {
      gameObjects.push_back(go_((new Enemy2(Vector(randomLike(i*33)*150-75,-1100-i*80,0),true))->circularShots()));
      gameObjects.push_back(go_((new Enemy2(Vector(randomLike(i*33)*150-75,-1100-i*80-40,0),false))->circularShots()));
    }
  }

  for (i = 0; i < 10; i++) {
    double xp = 75;
      gameObjects.push_back(go_(new LevelObject(Vector(-xp,-1100-i*5,0), Vector(0,1,0,0), object1)));
      gameObjects.push_back(go_(new LevelObject(Vector(xp,-1100-i*5,0), Vector(0,1,0,180), object1)));
  }

  for (j = 0; j < 2; j++) {
    for (int k = 0; k < 3; ++k) {
      double r = 1.0-(double)k/3;
      for (i = 0; i < 5; i++) {
        double xp = 75;
        gameObjects.push_back(go_(new LevelObject(Vector((-80+j*160)*r,-500-i*8-k*100,-xp), Vector(0,1,0,0-90), object1)));
        gameObjects.push_back(go_(new LevelObject(Vector((-80+j*160)*r,-500-i*8-k*100,xp), Vector(0,1,0,180-90), object1)));
      }
    }
  }

  for (j = 0; j < 2; j++) {
   for (i = 0; i < (j==0?8:18); i++) {
      double xp = 95;
      double a = i * 20;
      double a2 = a * PI / 180.0;
      a += 90;
      double lp = -100 - j * 1240;
      gameObjects.push_back(go_(new LevelObject(Vector(sin(a2)*xp,lp-i*4,cos(a2)*xp*0.2), Vector(0,1,0,0+a), object1)));
      gameObjects.push_back(go_(new LevelObject(Vector(sin(a2+PI)*xp,lp-i*4,cos(a2+PI)*xp*0.2), Vector(0,1,0,180+a), object1)));
    }
  }

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
    if (isKeyPressed(SCANCODE_SPACE)) {
      shot();
    }      
    if (isKeyPressed(SCANCODE_1)) {
      if (playerBlue) placeExplosionRing(playerPos);
      playerBlue = false;
    }      
    if (isKeyPressed(SCANCODE_2)) {
      if (!playerBlue) placeExplosionRing(playerPos);
      playerBlue = true;
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
