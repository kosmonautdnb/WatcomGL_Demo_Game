#ifndef __COLISION_HPP__
#define __COLISION_HPP__

#include "vector.hpp"

#define CAPSULE_PLAYER 0
#define CAPSULE_PLAYERSHOT 1
#define CAPSULE_COLLIDER 2
#define CAPSULE_ANOTHER_COLLIDER 3
#define MAX_CAPSULES 4

class Capsule {
public:
  Capsule() {radius = 0; initialized = false;}
  Capsule(const Vector &tip, const Vector &base, double radius) : tip(tip), base(base), radius(radius) {initialized = true;}
  bool initialized;
  Vector tip,base;
  double radius;
};

extern Capsule capsule[MAX_CAPSULES];

bool collide(const Capsule &a, const Capsule &b);
bool collide(int capsuleIndexA, int capsuleIndexB);

#endif //__COLISION_HPP__
