#include "config.hpp"
#include "sprite.hpp"

Mesh *player = NULL;
Mesh *enemy1 = NULL; // round/circular object 
Mesh *enemy2 = NULL; // ship red wings
Mesh *enemy3 = NULL; // a white black ball
Mesh *enemy4 = NULL; // a ball mine
Mesh *boss1 = NULL; // an endboss enemy
Mesh *collect = NULL; // a collectable object
Mesh *heart = NULL; // a energy fillup heart
Mesh *object1 = NULL; // hangar like <
Mesh *object2 = NULL; // grey plate with red stripe
Mesh *object3 = NULL; // a connector structure

#define CAPSULE_PLAYER_RADIUS 2
#define CAPSULE_ENEMYSHOT_RADIUS 2

void placeSmallSmoke(const Vector &p);
void placeEmitExplosion(const Vector &p);
void placeTailExplosion(const Vector &p);
void placeExplosionRing(const Vector &p);
void placeExplosion(const Vector &p);
void playerShotHitObject(const Vector &shotPos, GO *o);
bool isInScreen(const Vector &position);
void playerHit(bool explosion);
void enemyEnergyBar(double energy);

void drawMesh(Mesh *mesh) {
  glDisable(GL_TEXTURE_2D);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
  float c[4];
  c[0]=1;
  c[1]=1;             
  c[2]=1;
  c[3]=0;
  glLightfv(GL_LIGHT0,GL_SPECULAR,c);
  glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,c);
  c[0]=20;
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
    uint32_t col = f->color;
    if (reColor.has(col)) col = reColor[col];
    if ((col>>24)==0x40) {
      glDisable(GL_LIGHTING);
      col = 0xffff0000;
      col |= (int)(randomLike(currentTime*200)*0xff)<<8;
    } else {
      glEnable(GL_LIGHTING);
    }
    if ((col>>24)==0x20) {
      col = playerRed ? 0xff0000ff:0xffff00ff;
    }
    glColor4ubv((unsigned char*)&col);
    if (markDebug) glColor4f(1.0,0.0,1.0,1.0);
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
    active = true;
    this->size = size;
    this->smoke = smoke;
    this->black = black;
  }
  virtual void paint(double dt) {
    glExplicitAlpha(true,0.55);
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
      glTexCoord2f(tx[i],1-ty[i]);
      glVertex3f(p.x,p.y,p.z);
    }
    glEnd();
    glPopMatrix();
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glExplicitAlpha(true,0);
  }
};

class ExplosionRing : public GO, public GO_Position, public GO_LifeTime, public GO_Paintable {
public:
  bool _slow;
  ExplosionRing(const Vector &p) : GO(), GO_Position(p), GO_LifeTime(0.5), GO_Paintable() {
   _slow = false;
  }
  virtual void paint(double dt) {
    glExplicitAlpha(true,1);
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
    glExplicitAlpha(true,0);
  }
  ExplosionRing *slow() {_slow=true;lifeTime*=2;initialLifeTime*=2;return this;}
};

class ExplosionFlare : public GO, public GO_Position, public GO_LifeTime, public GO_Paintable {
public:
  ExplosionFlare(const Vector &p) : GO(), GO_Position(p), GO_LifeTime(0.5), GO_Paintable() {
  }
  virtual void paint(double dt) {
    glExplicitAlpha(true,1);
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
    glExplicitAlpha(true,0);
  }
};

class PlayerTurbineParticle : public GO, public GO_LifeTime, public GO_Paintable {
public:
  Vector displace;
  bool white;
  PlayerTurbineParticle(const Vector &p, bool white) : displace(p), white(white), GO(), GO_LifeTime(0.5), GO_Paintable() {
    active = true;
    static unsigned int k = 0; k++;
    lifeTime+=randomLike(k*111)*0.25;
    debugCountIt=false;
  }
  virtual void paint(double dt) {
    double l = (double)(initialLifeTime-lifeTime)/initialLifeTime;
    Vector position = playerPos + displace + Vector(0,10,0);
    position.y += l * 20;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, glowTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glDepthMask(GL_FALSE);
    float pointSize = XRES*4/320;
    glPointSize(pointSize);
    glBegin(GL_POINTS);
    double k = 1 - l;
    float r = 0.25*k;
    float g = 0.85*k;
    float b = 1.0*k;
    if (white) {
      r = g = b = k;
    }
    glColor3f(r,g,b);
    if (!USE_SPRITES) {
      glVertex3f(position.x, position.y, position.z);
    } else {
      int rg = r * 255;
      int gg = g * 255;
      int bg = b * 255;
      rg = clamp(rg,0,255);
      gg = clamp(gg,0,255);
      bg = clamp(bg,0,255);
      unsigned int color = rg|(gg<<8)|(bg<<16)|0xff000000;
      drawSprite(Vector(position.x,position.y,position.z),pointSize,pointSize,glowTexture,color,SPRITEFLAG_NODEPTHWRITE|SPRITEFLAG_ADDITIVE|SPRITEFLAG_BYSCREENSIZE);
    }
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDepthMask(GL_TRUE);
  }
  static void emit() {
    static double t = 0;
    t += timeDelta;
    if (fabs(t)>0.0125) {
      for (int i = 0; i < 4; i++) {
        static unsigned int k=0; k++;
        t = 0;
        double r = 1.0;
        double x = 0;
        double y = 0;
        gameObjects.push_back(go_(new PlayerTurbineParticle(Vector(x+(randomLike(k*33)*2-1)*r,y+(randomLike(k*33)*2-1)*r,0),(k&4)&&(k&8)&&(k&2))));
      }
    }
  }
};

class PlayerShot : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance {
public:
  Vector lastPosition;
  bool fresh;
  PlayerShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180) {
    active = true;
    position = p;
    velocity = v;
    fresh = true;
  }
  virtual void paint(double dt) {
    if (isInScreen(position)) {
      if (!fresh) {
        capsule[CAPSULE_PLAYERSHOT] = Capsule(position, lastPosition, CAPSULE_PLAYER_RADIUS);
        for (int i = 0; i < gameObjects.size(); i++) {
          GO *o = go_(gameObjects[i]);
          if (!__RUNNING(o)) continue;
          GO_Collider_Enemy *v0 = dynamic_cast<GO_Collider_Enemy*>(o);
          if (v0 != NULL && v0->collideWithCapsule(CAPSULE_PLAYERSHOT)) {
            playerShotHitObject(position,o);
            destruct();
          }
          GO_Collider_LevelObject *v1 = dynamic_cast<GO_Collider_LevelObject*>(o);
          if (v1 != NULL && v1->collideWithCapsule(CAPSULE_PLAYERSHOT)) {
            destruct();
          }
        }
      }
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, shotTexture[2]);
      glBegin(GL_POINTS);
      glEnable(GL_BLEND);
      glDepthMask(GL_FALSE);
      glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA); // premultiplied alpha for add transparencies
      glColor4f(1,1,1,1);
      const float pointSize = XRES*10/320;
      if (!USE_SPRITES) {
        glPointSize(pointSize);
        glVertex3f(position.x,position.y,position.z);
      } else {
        drawSprite(Vector(position.x,position.y,position.z),pointSize,pointSize,shotTexture[2],0xffffffff,SPRITEFLAG_PREMULTIPLIED_ALPHA|SPRITEFLAG_NODEPTHWRITE|SPRITEFLAG_BYSCREENSIZE);
      }
      glEnd();
      glDepthMask(GL_TRUE);
      glDisable(GL_TEXTURE_2D);
      glDisable(GL_BLEND);
    }
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
  bool red;
  Vector lastPosition;
  bool fresh;
  EnemyShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180), GO_LifeTime(100) {
    active = true;
    initialPosition = p;
    position = p;
    velocity = v;
    red = false;
    fresh = true;
  }
  virtual void paint(double dt) {
    capsule[CAPSULE_COLLIDER] = Capsule(position, lastPosition, CAPSULE_ENEMYSHOT_RADIUS);
    if ((red==playerRed) && (fresh == false) && collide(CAPSULE_COLLIDER,CAPSULE_PLAYER)) {
      destruct();
      placeExplosion(position);
      playerHit(false);
    }
    Vector position2 = position;
    glEnable(GL_TEXTURE_2D);
    unsigned int texture = shotTexture[red?0:1];
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.1f);
    glDepthMask(GL_FALSE);
    glBegin(GL_POINTS);
    glColor4f(1,1,1,1);
    float pointSize = XRES*5/320;
    glPointSize(pointSize);
    if (!USE_SPRITES) {
      glVertex3f(position2.x,position2.y,position2.z);
    } else {
      drawSprite(Vector(position2.x,position2.y,position2.z),pointSize,pointSize,texture,0xffffffff,SPRITEFLAG_NODEPTHWRITE|SPRITEFLAG_BYSCREENSIZE);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_TRUE);
    lastPosition = position;
    fresh = false;
  }
  virtual void destruct() {
    placeEmitExplosion(position);
    GO::destruct();
  }
};

class EnemyShot2 : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance, public GO_LifeTime {
public:
  Vector initialPosition;
  bool red;
  Vector lastPosition;
  bool fresh;
  EnemyShot2(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180), GO_LifeTime(100) {
    active = true;
    initialPosition = p;
    position = p;
    velocity = v;
    red = false;
    fresh = true;
  }
  Vector rotate(const Vector &v, double ang) {
    ang = ang * PI / 180.0;
    Vector r;
    r.x = cos(ang)*v.x-sin(ang)*v.y;
    r.y = sin(ang)*v.x+cos(ang)*v.y;
    r.z = v.z;
    return r;
  }
  virtual void paint(double dt) {
    capsule[CAPSULE_COLLIDER] = Capsule(position, lastPosition, CAPSULE_ENEMYSHOT_RADIUS);
    if ((red==playerRed) && (fresh == false) && collide(CAPSULE_COLLIDER,CAPSULE_PLAYER)) {
      destruct();
      placeExplosion(position);
      playerHit(false);
    }
    Vector position2 = position;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, shotTexture[red?4:5]);
    //glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_GREATER,0.1f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
    double shotSize = 4;
    double k = seconds*180.0+randomLike(index*100)*90;
    Vector n = rotate(Vector(0,3,0)*shotSize, k);
    Vector t = rotate(Vector(1.0,0,0)*shotSize, k);
    glDepthMask(GL_FALSE);
    glBegin(GL_QUADS);
    glColor4f(1,1,1,1);
    glTexCoord2f(1,1);
    glVertex3f(position2.x+t.x-n.x,position2.y+t.y-n.y,position2.z+t.z-n.z);
    glTexCoord2f(0,1);
    glVertex3f(position2.x-t.x-n.x,position2.y-t.y-n.y,position2.z-t.z-n.z);
    glTexCoord2f(0,0);
    glVertex3f(position2.x-t.x+n.x,position2.y-t.y+n.y,position2.z-t.z+n.z);
    glTexCoord2f(1,0);
    glVertex3f(position2.x+t.x+n.x,position2.y+t.y+n.y,position2.z+t.z+n.z);
    glEnd();
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_ALPHA_TEST);
    glDepthMask(GL_TRUE);
    lastPosition = position;
    fresh = false;
  }
  virtual void destruct() {
    placeEmitExplosion(position);
    GO::destruct();
  }
};

class LevelObject : public GO, public GO_Position, public GO_Paintable, public GO_Rotation, public GO_Collider_LevelObject {
public:
  Mesh *mesh;
  bool _randomRotate;
  Vector _scale;
  Vector _rotation;
  LevelObject(const Vector &p, const Vector &r, Mesh *mesh) : GO(), GO_Position(p), GO_Paintable(), GO_Rotation(r.w,r.xyz()), GO_Collider_LevelObject(10) {
    this->mesh = mesh;
    _randomRotate = false;
    _scale = Vector(1,1,1,1);
    _rotation = Vector(0,0,1,0);
  }
  virtual void paint(double dt) {
    glPushMatrix();
    position.y += dt * 30.0;
    glTranslatef(position.x,position.y,position.z);
    if (_randomRotate) {
      Vector v = Vector(randomLike(index*10),randomLike(index*20),randomLike(index*30));
      glRotatef(seconds*45,v.x,v.y,v.z);
    }
    glRotatef(angle,axis.x,axis.y,axis.z);
    glRotatef(_rotation.x,_rotation.y,_rotation.z,_rotation.w);
    glScalef(_scale.x,_scale.y,_scale.z);
    markDebug = debugMark;
    drawMesh(mesh);
    markDebug = false;
    glPopMatrix();
  }
  LevelObject *randomRotate() {_randomRotate=true;return this;}
  LevelObject *scale(const Vector &_scale) {this->_scale=_scale;return this;}
  LevelObject *rotate(const Vector &_rotation) {this->_rotation=_rotation;return this;}
};

class Collectable : public GO, public GO_Position, public GO_Paintable, public GO_Rotation, public GO_Collider_LevelObject {
public:
  bool _heart;
  bool _weaponGreen;
  Collectable(const Vector &p) : GO(), GO_Position(p), GO_Paintable(), GO_Rotation(), GO_Collider_LevelObject(10) {
    _heart = false;
    _weaponGreen = false;
  }
  virtual void paint(double dt) {
    glExplicitAlpha(true,1);
    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);
    float l = sin(seconds*2*PI*1.5)*0.5f+0.75f;
    int r = (int)clamp(l*55*2,0.f,255.f);
    int g = (int)clamp(l*255.f*2,0.f,255.f);
    int b = (int)clamp(l*155.f*2,0.f,255.f);
    reColor[0xff00ff00] = r|(g<<8)|(b<<16)|0xff000000;
    if (_weaponGreen) {
      glRotatef(seconds*120,sin(seconds),cos(seconds),0);
      glScalef(0.75,0.75,0.625);
      glRotatef(seconds*90,0,0,1);
    }
    if (_heart) {
      glRotatef(-60,1,0,0);
      glRotatef(seconds*90,0,0,1);
    }
    drawMesh(_heart ? heart : collect);
    reColor.clear();
    glPopMatrix();
    glExplicitAlpha(true,0);
  }
  virtual void collidedWithPlayer() {
    placeExplosionRing(playerPos);
    deleteIt = true;
    if (_weaponGreen) {
      weaponColor = WEAPON_GREEN;
      weaponStrength++;
      weaponSample->play(position);
    }
    if (_heart) {
      playerHits = 0;
      powerUpSample->play(position);
    }
    slideSound->play(position);
  }
  Collectable *weaponGreen() {_weaponGreen=true; return this;}
  Collectable *life() {_heart=true; return this;}
};

void placeSmallSmoke(const Vector &p) {
  gameObjects.push_back(go_(new Explosion(p,Vector(),2,1.0,true,false)));
  //shotSound->play(p);
}

void placeEmitExplosion(const Vector &p) {
  gameObjects.push_back(go_(new Explosion(p,Vector(),3,0.75,false,false)));
}

void placeTailExplosion(const Vector &p) {
  static unsigned int k = 0; k++; k &= 0xffff;
  double a = k*2*PI/20;
  double sp = 10.0;
  gameObjects.push_back(go_(new Explosion(p,Vector(sin(a)*sp,cos(a)*sp,0),randomLike(k*33)*2.5+1.5,1.5,false,false)));
}

void placeExplosionRing(const Vector &p) {
  gameObjects.push_back(go_(new ExplosionRing(p)));
}

void placeExplosion(const Vector &p) {
  static unsigned int k;
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
  k &= 0xffff;
}

void playerShotHitObject(const Vector &shotPos, GO *o) {
  if (!__RUNNING(o)) return;

  GO_HitPoints *v0 = dynamic_cast<GO_HitPoints*>(o);
  if (v0 != NULL) {
    v0->subtractHitPoints(1);
    placeSmallSmoke(shotPos);
  }
}

bool isInScreen(const Vector &position) {
  const double y = position.y + levelScrollY;
  if (y > -65.0 && y < 65.0) return true;
  return false;
}

void playerHit(bool explosion) {
  if (playerLifeLostAnimDuration>0) return;
  playerHits++;
  if (explosion)
    placeExplosion(playerPos);
  if (playerHits>=maxPlayerHits)
    liveLost = true;
}

void enemyEnergyBar(double energy) {
  hudEnergyBarValue = energy;
  hudEnergyBarDuration = 1;
}
