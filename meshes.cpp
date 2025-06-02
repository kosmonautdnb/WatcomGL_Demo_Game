#include "meshes.hpp"
#include "smplobjl.hpp"
#include "gl.h"

static unsigned int translateToColor(const String &c) {
  String k = toLower(c);
  if (k.startsWith("white")) return 0xffffffff;
  if (k.startsWith("grey")) return 0xff808080;
  if (k.startsWith("red")) return 0xff0000ff;
  if (k.startsWith("black")) return 0xff000000;
  if (k.startsWith("plexiglas")) return 0x80ffc000;
  if (k.startsWith("lightblue")) {return 0x20000000;}
  if (k.startsWith("turbine")) return 0x4000c0ff;
  //if (k.startsWith("color1")) return 0xffffffff;
  //if (k.startsWith("color2")) return 0xff0000ff;
  glDone();
  printf("%s\n",k.c_str());
  exit(0);
  return 0;
}

Vector swizzle(const Vector &v) {
  return Vector(v.z,-v.x,-v.y);
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

