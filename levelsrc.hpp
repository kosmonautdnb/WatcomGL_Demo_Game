Mesh *player;
Mesh *enemy1; // round/circular object 
Mesh *enemy2; // ship red wings
Mesh *enemy3; // a white black ball
Mesh *enemy4; // a ball mine
Mesh *collect; // a collectable object
Mesh *object1; // hangar like <
Mesh *object2; // grey plate with red stripe

#define CAPSULE_PLAYER_RADIUS 2

void placeSmallSmoke(const Vector &p);
void placeEmitExplosion(const Vector &p);
void placeTailExplosion(const Vector &p);
void placeExplosionRing(const Vector &p);
void placeExplosion(const Vector &p);
void playerShotHitObject(const Vector &shotPos, GO *o);
bool isInScreen(const Vector &position);
void playerHit(bool explosion);

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
      col = (!playerBlue) ? 0xff0000ff:0xffff00ff;
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
    activated = true;
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
  bool white;
  PlayerTurbineParticle(const Vector &p, bool white) : displace(p), white(white), GO(), GO_LifeTime(0.5), GO_Paintable() {
    activated = true;
    static int k = 0; k++;
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
    glPointSize(XRES*4/320);
    glBegin(GL_POINTS);
    double k = 1 - l;
    glColor3f(0.25*k,0.85*k,1.0*k);
    if (white)
      glColor3f(k,k,k);
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
    activated = true;
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

class EnemyShot : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance, public GO_LifeTime {
public:
  Vector initialPosition;
  bool blue;
  Vector lastPosition;
  bool fresh;
  EnemyShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180), GO_LifeTime(100) {
    activated = true;
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

class LevelObject : public GO, public GO_Position, public GO_Paintable, public GO_Rotation, public GO_Collider_LevelObject {
public:
  Mesh *mesh;
  bool _randomRotate;
  Vector _scale;
  LevelObject(const Vector &p, const Vector &r, Mesh *mesh) : GO(), GO_Position(p), GO_Paintable(), GO_Rotation(r.w,r.xyz()), GO_Collider_LevelObject(10) {
    this->mesh = mesh;
    _randomRotate = false;
    _scale = Vector(1,1,1,1);
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
    glScalef(_scale.x,_scale.y,_scale.z);
    markDebug = debugMark;
    drawMesh(mesh);
    markDebug = false;
    glPopMatrix();
  }
  LevelObject *randomRotate() {_randomRotate=true;return this;}
  LevelObject *scale(const Vector &_scale) {this->_scale=_scale;return this;}
};

class Collectable : public GO, public GO_Position, public GO_Paintable, public GO_Rotation, public GO_Collider_LevelObject {
public:
  Collectable(const Vector &p) : GO(), GO_Position(p), GO_Paintable(), GO_Collider_LevelObject(10) {
  }
  virtual void paint(double dt) {
    glPushMatrix();
    glTranslatef(position.x,position.y,position.z);
    glRotatef(seconds*120,sin(seconds),cos(seconds),0);
    float l = sin(seconds*2*PI*1.5)*0.5f+0.75f;
    int r = (int)clamp(l*55,0.f,255.f);
    int g = (int)clamp(l*255.f,0.f,255.f);
    int b = (int)clamp(l*155.f,0.f,255.f);
    reColor[0xff00ff00] = r|(g<<8)|(b<<16)|0xff000000;
    glScalef(1,1,0.75);
    glRotatef(seconds*90,0,0,1);
    drawMesh(collect);
    reColor.clear();
    glPopMatrix();
  }
  virtual void collidedWithPlayer() {
    placeExplosionRing(playerPos);
    deleteIt = true;
    weaponColor = WEAPON_GREEN;
    weaponStrength++;
    slideSound->play(position);
  }
};

void placeSmallSmoke(const Vector &p) {
  gameObjects.push_back(go_(new Explosion(p,Vector(),2,1.0,true,false)));
  //shotSound->play(p);
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
  if (o->deleteIt)
    return;
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

void playerHit(bool explosion) {
  playerHits++;
  if (explosion)
    placeExplosion(playerPos);
}




