#ifndef __MESHES_HPP__
#define __MESHES_HPP__

#include "array.hpp"
#include "vector.hpp"
#include "string.hpp"

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

Mesh *loadObject(const String &fileName);
void centerAndResizeObject(Mesh *mesh, double size);

#endif //__MESHES_HPP__
