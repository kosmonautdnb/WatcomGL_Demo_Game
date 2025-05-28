#include "gl.h"
#include "types.hpp"
#include "keymtrix.hpp"
#include "array.hpp"
#include "vector.hpp"
#include "object.hpp"
#include "string.hpp"
#include "speaker.hpp"
#include "smplobjl.hpp"
#include <math.h>
#define PI 3.1415926
#include "gameobj.hpp"
#include "gameomgr.hpp"
#include "image.hpp"

#define XRES 320
#define YRES 200

class Face {
public:
  int v[3];
  int n[3];
  unsigned int color;
};

class Mesh {
public:
  Array<Vector> vertices;
  Array<Vector> normals;
  Array<Face> faces;
};

class Sample {
public:
  unsigned char *sample;
  int sampleLength;

  void play() {
    playSample(sample, sampleLength, false);
  }
};

Mesh *player = NULL;
Mesh *enemy = NULL;
Sample *shotSound = NULL;

template<class T>
T min(const T&a, const T&b) {
  return a<b?a:b;
}

template<class T>
T max(const T&a, const T&b) {
  return a>b?a:b;
}

template<class T>
T clamp(const T&a, const T &b, const T &c) {
  return a<b?b:(a>c?c:a);
}

template<class T>
T saturate(const T&a) {
  return clamp(a,(T)0,(T)1);
}

static const float randomLike(const int index) {
    int b = index ^ (index * 11) ^ (index / 17) ^ (index >> 16) ^ (index * 1877) ^ (index * 8332) ^ (index * 173);
    b = b ^ (b << 8) ^ (b * 23);
    b >>= 3;
    return (float)(b & 0xffff) / 0x10000;
}

const float rand2d(int x, int y) {
  return randomLike((x*17)+randomLike(y*31)*0x4000);
}

const float rand2dp(double x, double y) {
  const int xi = (int)floor(x);
  const int yi = (int)floor(y);
  double fx = x - (double)xi;
  double fy = y - (double)yi;
  fx*=fx;
  fy*=fy;
  const double p00 = rand2d(xi,yi);
  const double p10 = rand2d(xi+1,yi);
  const double p11 = rand2d(xi+1,yi+1);
  const double p01 = rand2d(xi,yi+1);
  const double top = (p10 - p00) * fx + p00;
  const double btm = (p11 - p01) * fx + p01;
  return (btm-top)*fy+top;
}

const float perlin(double x, double y) {
  float r = 0;
  r += rand2dp(x*1,y*1)/1.0;
  r += rand2dp(x*2,y*2)/2.0;
  r += rand2dp(x*4,y*4)/4.0;
  r += rand2dp(x*8,y*8)/4.0;
  //r += rand2dp(x*16,y*16)/16.0;
  return r;
}

void loadObjectFileCallback(void *ctx, const char *filename, int is_mtl, const char *obj_filename, char **buf, size_t *len) {
  *buf = NULL;
  *len  = 0;
  FILE *in = fopen(filename,"rb");
  if (in == NULL) return;
  fseek(in,0,SEEK_END);
  *len = ftell(in);
  fseek(in,0,SEEK_SET);
  *buf = (char*)malloc(*len);
  fread(*buf,1,*len,in);
  fclose(in);
}

unsigned int translateToColor(const String &c) {
  String k = toLower(c);
  if (k.startsWith("white")) return 0xffffffff;
  if (k.startsWith("red")) return 0xff0000ff;
  if (k.startsWith("plexiglas")) return 0x80ffc000;
  if (k.startsWith("lightblue")) return 0xffff8080;
  if (k.startsWith("turbine")) return 0x4000c0ff;
  if (k.startsWith("color1")) return 0xffffffff;
  if (k.startsWith("color2")) return 0xff0000ff;
  glDone();
  printf("a%da\n",k.c_str()[6]);
  exit(0);
  return 0;
}

Vector swizzle(const Vector &v) {
  return Vector(v.x,-v.y,-v.z);
}

Mesh *loadObject(const String &fileName) {
  Mesh *ret = NULL;
  SMPL_File *f = loadObj(fileName,true);
  if (f == NULL) {
    glDone();
    printf("ReadError\n");
    exit(0);
  }
  ret = new Mesh();
  int i;
  for (i = 0; i < f->vertices.size(); i++) {
    ret->vertices.push_back(swizzle(f->vertices[i]));
  }
  for (i = 0; i < f->normals.size(); i++) {
    ret->normals.push_back(swizzle(f->normals[i]));
  }
  for (i = 0; i < f->objs.size(); i++) {
    SMPL_Object *o = &f->objs[i];
    for (int j = o->faceStart; j < o->faceEnd; j++) {
      SMPL_Face *f2 = &f->faces[j];
      ret->faces.push_back(Face());
      Face *t = &ret->faces.back();
      if (f2->c != 3) {glDone(); printf("error%d\n",f2->c);exit(0);}
      t->v[0] = f2->v[0];
      t->v[1] = f2->v[1];
      t->v[2] = f2->v[2];
      t->n[0] = f2->n[0];
      t->n[1] = f2->n[1];
      t->n[2] = f2->n[2];
      //t->color = translateToColor(f->materialNames[f2->m]);
      t->color = translateToColor(o->name);
    }
  }  
  delete f;
  return ret;
}

void centerAndResizeObject(Mesh *mesh, double size) {
  int i;
  Vector center;
  for (i = 0; i < mesh->vertices.size(); ++i) {
    center+=mesh->vertices[i];
  }
  center/=mesh->vertices.size();
  double radius = 0;
  for (i = 0; i < mesh->vertices.size(); ++i) {
    double r = length(mesh->vertices[i]-center);
     if (r > radius) radius = r;
  }


  for (i = 0; i < mesh->vertices.size(); ++i) {
    mesh->vertices[i] = (mesh->vertices[i]-center)*size/radius;
  }  
}

Sample *loadSample() {
  Sample *ret=NULL;
  unsigned int len = speakerFrequency*3;
  unsigned short *sample = (unsigned short*)malloc(len*sizeof(unsigned short));
  for (int i = 0; i < len; i++) {
    double ri = (double)i / len;
    ri = pow(ri,1.5);
    float f = sin(ri*2.0*PI*0.5);
    f = clamp(f,-1.f,1.f);
    sample[i]=f*32000+32768;
  }
  ret = new Sample();
  ret->sample = convertSampleFromUInt16(sample,len);
  ret->sampleLength = (len+7)/8;
  free(sample);
  return ret;
}

GLuint glowTexture = 0;
GLuint cloudTexture = 0;
unsigned int buffer[128*128];

void loadTextures() {
  int x,y;

  for (y = 0; y < 32; y++) {
    float fy = y / 31.f * 2.f - 1.f;
    for (x = 0; x < 32; x++) {
      float fx = x / 31.f * 2.f - 1.f;
      float d = 1.f-sqrt(fx*fx+fy*fy);
      if (d < 0) d = 0;
      d = pow(d,2);
      int r = d * 255.f;
      if (r < 0) r = 0;
      if (r > 255) r = 255;
      buffer[x+y*32] = r | (r<<8) | (r<<16) | 0xff000000;
    }
  }
  glGenTextures(1, &glowTexture);
  glBindTexture(GL_TEXTURE_2D, glowTexture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,32,32,0,GL_RGBA,GL_UNSIGNED_BYTE,buffer);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


  RGBAImage img = RGBAImage::fromFile("big3.png");
  glGenTextures(1, &cloudTexture);
  glBindTexture(GL_TEXTURE_2D, cloudTexture);
  glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,img.width,img.height,0,GL_RGBA,GL_UNSIGNED_BYTE,img.data);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

#define FOV 30
#define ASPECT (16.0/9.0)
#define NEARPLANE 0.1
#define FARPLANE 1000.0
#define CAMPOSZ -120.0
#define CAMPOSY 150.0
#define CAMLOOKZ 1.0
double lastTime = 0;
double currentTime = 0;
double timeDelta = 0;
double scrollPosX=0;
double scrollPosY=0;
double walkingAngle=0;
Vector playerPos;
double playerSpeed = 50.0;
double playerRoll = 0;
double playerDestRoll = 0;
double playerRollSpeed = 100.0;


void setCamera() {
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(FOV,ASPECT,NEARPLANE,FARPLANE);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(0,CAMPOSY,CAMPOSZ,0,0,0,0,-1,0);
}

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
  c[0] = 1;
  c[1] = -1;
  c[2] = -1;
  c[3] = 0; // directional
  glLightfv(GL_LIGHT0,GL_POSITION,c);
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
  glPopMatrix();
}

double levelScrollY = 0;
double levelScrollX = 0;

void drawEnemy(Mesh *mesh, const Vector &pos) {
  glPushMatrix();
  glTranslatef(pos.x-levelScrollX,pos.y,pos.z);
  glRotatef(currentTime*80,sin(currentTime),cos(currentTime),0);
  drawMesh(mesh);
  glPopMatrix();
}

class PlayerShot : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance {
public:
  Vector lastPosition;
  bool fresh;
  PlayerShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180) {
    position = p;
    velocity = v;
    fresh = true;
    shotSound->play();
  }
  virtual void paint() {
    Vector delta = normalize(position-lastPosition)*2;
    lastPosition = position;
    if (fresh) {fresh = false; return;}
    glBegin(GL_LINES);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glDisable(GL_BLEND);
    float r = 1.0;
    float g = 0.5;
    float b = 0.3;
    glColor3f(r*0.5,g*0.5,b*0.5);
    glLineWidth(XRES*3/640);
    glVertex3f(position.x,position.y,position.z);
    glVertex3f(position.x-delta.x*2,position.y-delta.y*2,position.z-delta.z*2);
    glDisable(GL_BLEND);
    glLineWidth(XRES*2/640);
    glColor3f(r,g,b);
    glVertex3f(position.x,position.y,position.z);
    glVertex3f(position.x-delta.x,position.y-delta.y,position.z-delta.z);
    glEnd();
  }
  PlayerShot *vorlauf(double dt) {
    position += velocity * dt;
    return this;
  }
};

class EnemyShot  : public GO, public GO_Position, public GO_Physical, public GO_Paintable, public GO_AliveDistance, public GO_LifeTime {
public:
  Vector initialPosition;
  Vector lastPosition;
  bool fresh;
  bool blue;
  EnemyShot(const Vector &p, const Vector &v) : GO(), GO_Position(), GO_Physical(), GO_Paintable(), GO_AliveDistance(180), GO_LifeTime(100) {
    initialPosition = p;
    position = p;
    velocity = v;
    fresh = true;
    blue = false;
  }
  virtual void paint() {
    Vector position2 = position;
    Vector z = position2 - initialPosition;
    float ang = atan2(z.x,z.y) + lifeTime*0;
    float dist = length(z);
    position2 = Vector(sin(ang)*dist,cos(ang)*dist) + initialPosition;
    Vector delta = normalize(position2-lastPosition)*2;
    lastPosition = position2;
    position2.x -= levelScrollX;
    if (fresh) {fresh = false; return;}
    glBegin(GL_LINES);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glDisable(GL_BLEND);
    float r = 1.0;
    float g = 0.0;
    float b = 0.0;
    if (blue) {
      r = 1.0;
      g = 0.0;
      b = 1.0;
    }
    glColor3f(r*0.5,g*0.5,b*0.5);
    glLineWidth(XRES*3/640);
    glVertex3f(position2.x,position2.y,position2.z);
    glVertex3f(position2.x-delta.x*2,position2.y-delta.y*2,position2.z-delta.z*2);
    glDisable(GL_BLEND);
    glLineWidth(XRES*2/640);
    glColor3f(r,g,b);
    glVertex3f(position2.x,position2.y,position2.z);
    glVertex3f(position2.x-delta.x,position2.y-delta.y,position2.z-delta.z);
    glEnd();
  }
};

class SimpleCircleShotEmitter : public GO, public GO_Position, public GO_FrequencyCallback, public GO_Callback {
public:
  int sectors;
  double shotSpeed;
  SimpleCircleShotEmitter(const Vector &p, double frequency, double shotSpeed, int sectors) : GO(), GO_Position(p), GO_FrequencyCallback(frequency), GO_Callback() {
    deleteIt = false; // actually this should have been set by GO()
    this->sectors = sectors;
    this->shotSpeed = shotSpeed;
  }
  virtual void frequent(int iteration) {
    float speed = 10.0;
    for (int i = 0; i < sectors; i++) {
      float a = i * 2 * PI / sectors + iteration;
      Vector dir(sin(a)*shotSpeed,cos(a)*shotSpeed,0);
      GO *enemyShot = new EnemyShot(position,dir);
      if ((iteration % 12)==0)
        dynamic_cast<EnemyShot*>(enemyShot)->blue=true;
      gameObjects.push_back(enemyShot);
    }
    if ((iteration % 12)==0)
      shotSound->play();
  }
  virtual void callback(double dt) {
    position.y+=dt*10;
  }
};

void shot() {
  static double lastShotTime = currentTime;
  double distance = fabs(lastShotTime - currentTime);
  if (distance > 0.2) {
    lastShotTime = currentTime;
    for (int i = -1; i <= 1; i++) {
      Vector k = playerPos;  
      k.x += i*3;
      gameObjects.push_back((GO*)(new PlayerShot(k,Vector(0,-80,0))));
      gameObjects.push_back((GO*)(new PlayerShot(k,Vector(-20,-80,0)))->vorlauf(i*0.025));
      gameObjects.push_back((GO*)(new PlayerShot(k,Vector(+20,-80,0)))->vorlauf(-i*0.025));
    }
  }
}

void paintLevel() {
  const double sc = 0.01;
  const double scale = 0.5;
  const double dY = 18.0*scale;
  const double fY =  fmod(levelScrollY,dY);
  const double tY = levelScrollY - fY;
  const double particleSize = 7.5;
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_ONE);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,glowTexture);
  glDepthMask(GL_FALSE);
  glBegin(GL_POINTS);
  glColor3f(1,1,1);
  int yNear = 5+2;
  int yFar = -25-23;
  int nearX = 34;
  int farX = 85;
  for (int y = yFar; y < yNear; y++) {
    int xHere = (farX-nearX)*abs(y-yNear)/abs(yFar-yNear)+nearX;
    double lp = 0;
    for (int x = -xHere/2; x <= xHere/2; x++) {
      Vector k;
      k.x = x * 3.75*2;
      k.y = y * dY + sin(k.x*0.1*0.25+(y*dY-tY)*0.025)*dY;
      double p = perlin(k.x*sc,(k.y-tY)*sc);
      k.y += fY;
      double d = 1.0+(lp-p)*10.0;
      if (d < 0) d  = 0;
      float r = 2-p*1.5;
      float g = 2-p;
      float b = p;
      float subi = 0.5;
      if (p>1.1) {
        r = 0.5;
        g = 0.75;
        b = 1;
      }
      k.z = p*120.0;
      if (k.z > 120) k.z = 120;
      d *= perlin(k.x*sc*2,(k.y-tY-fY-levelScrollY*4)*sc*2);
      glPointSize(XRES*(particleSize+randomLike(k.y-tY-fY+randomLike(k.x*13.0))*14.0)/640);
      glColor3f(d*r-subi,d*g-subi,d*b-subi);
      k.x -= levelScrollX*0.25;
      glVertex3f(k.x,k.y,k.z);
      lp = p;
    }
  }
  glEnd();
  glDepthMask(GL_TRUE);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
}

void paintCloudLayer() {
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
  //glDisable(GL_BLEND);
  glEnable(GL_ALPHA_TEST);
  glAlphaFunc(GL_GREATER,0.1f);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,cloudTexture);
  glDepthMask(GL_FALSE);
  glBegin(GL_QUADS);
  for (int i = 0; i < 5; i++) {
    double k = (levelScrollY+randomLike(i*54+1000)*1500.f)*(randomLike(i*33+7)*4.f+1.f);
    int r = (int)floor(k / 600.0);
    double yPos = fmod(k,600.f)-350;
    double xPos = randomLike(i*678+r*33)*300.f-150.f-levelScrollX*(0.5+randomLike(i*123+r*23)*0.5);
    double xSize = 80.0*randomLike(i*129+r*99)+50.0;
    double ySize = xSize*(randomLike(i*771+r*337)*1.5+0.5);
    double e = randomLike(i*33+7)*0.2+0.8;
    glColor4f(e,e,e,0.85f);
    glTexCoord2f(1,0);
    glVertex3f(xPos+xSize,yPos,0);
    glTexCoord2f(0,0);
    glVertex3f(xPos,yPos,0);
    glTexCoord2f(0,1);
    glVertex3f(xPos,yPos+ySize,0);
    glTexCoord2f(1,1);
    glVertex3f(xPos+xSize,yPos+ySize,0);
  }
  glColor4f(1,1,1,1);
  glEnd();
  glDepthMask(GL_TRUE);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_BLEND);
  glDisable(GL_ALPHA_TEST);
}

void paintLineLayer() {
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE,GL_ONE);
  glDepthMask(GL_FALSE);
  glBegin(GL_LINES);
  for (int i = 0; i < 100; i++) {
    double k = (levelScrollY+randomLike(i*54+1000)*1500.f)*(randomLike(i*33+7)*5.f+2.f);
    double yPos = fmod(k,500.f)-300;
    int r = (int)floor(k / 500.0);
    double xPos = randomLike(i*678+r*33)*300.f-150.f-levelScrollX*(0.375+randomLike(i*123+r*45)*0.4);
    double ySize = 5.0*randomLike(i*447+r*22)+5.0;
    glLineWidth(XRES*(randomLike(i*147+r*332)*3.0+1.0)/640);
    glColor4f(0,0,0,1);
    glVertex3f(xPos,yPos-ySize,0);
    glColor4f(0.2,0.6,0.6,0);
    glVertex3f(xPos,yPos+ySize,0);
  }
  glColor4f(1,1,1,1);
  glEnd();
  glDepthMask(GL_TRUE);
  glDisable(GL_BLEND);
}

int main(int argc, const char *argv) {
  installKeyboardHandler();
  glVesa(XRES,YRES,32);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  player = loadObject("player.obj");
  enemy = loadObject("impostor.obj");
  centerAndResizeObject(player,10.0);
  centerAndResizeObject(enemy,10.0);
  shotSound = loadSample();
  loadTextures();

  enableSamplePlayback();
  go_setupManagers();
   
  GO *a = new SimpleCircleShotEmitter(Vector(-100,-200,0),0.2,32,20);
  gameObjects.push_back(a);
  GO *b = new SimpleCircleShotEmitter(Vector(+100,-200,0),0.2,32,40);
  gameObjects.push_back(b);
  GO *c = new SimpleCircleShotEmitter(Vector(0,-400,0),0.8,32,80);
  gameObjects.push_back(c);

  bool done = false;
  do {
    currentTime = speakerSeconds;
    timeDelta = currentTime-lastTime;
    lastTime = currentTime;

    glClearColor(0.0,0.0,0.0,1.0);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    setCamera();

    levelScrollY += timeDelta * 10.0;
    paintLevel();
    paintLineLayer();
    paintCloudLayer();
    drawPlayer(player,playerPos);
    drawEnemy(enemy,dynamic_cast<GO_Position*>(a)->position);
    drawEnemy(enemy,dynamic_cast<GO_Position*>(b)->position);
    drawEnemy(enemy,dynamic_cast<GO_Position*>(c)->position);
    int currentKey = glNextKey();
    if (currentKey == GL_VK_ESCAPE) done = true;
                   
    if (playerDestRoll<playerRoll) playerRoll -= timeDelta*playerRollSpeed;
    if (playerDestRoll>playerRoll) playerRoll += timeDelta*playerRollSpeed;

    playerDestRoll = 0;
    if (isKeyPressed(SCANCODE_UP)) {
      playerPos.y -= playerSpeed*timeDelta;
    }
    if (isKeyPressed(SCANCODE_DOWN)) {
      playerPos.y += playerSpeed*timeDelta;
    }
    if (isKeyPressed(SCANCODE_LEFT)) {
      playerPos.x -= playerSpeed*timeDelta;
      playerDestRoll = +45;
    }
    if (isKeyPressed(SCANCODE_RIGHT)) {
      playerPos.x += playerSpeed*timeDelta;
      playerDestRoll = -45;
    }
    if (isKeyPressed(SCANCODE_RSHIFT)) {
      shot();
    }      
    playerPos.x = clamp(playerPos.x,-64.0,64.0);
    playerPos.y = clamp(playerPos.y,-100.0,50.0);
    levelScrollX = playerPos.x;

    go_onFrame(timeDelta);
    glRefresh();
  } while(!done);
  disableSamplePlayback();
  uninstallKeyboardHandler();
  glDone();
  return 0;
}
