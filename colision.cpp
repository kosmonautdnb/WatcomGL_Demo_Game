#include "colision.hpp"
#include "util.hpp"

Capsule capsule[MAX_CAPSULES];

// wickedengine.net
Vector ClosestPointOnLineSegment(const Vector &A, const Vector &B, const Vector &Point) {
  Vector AB = B - A;
  double t = dot(Point-A,AB);
  return A + saturate(t) * AB;
}

// wickedengine.net
bool collide(const Capsule &a, const Capsule &b) {
  if ((!a.initialized) || (!b.initialized))
    return false;
  Vector ad = a.tip-a.base;
  Vector bd = b.tip-b.base;
  double al = length(ad);
  double bl = length(bd);
  double ra = al < a.radius ? a.radius : al;
  double rb = bl < b.radius ? b.radius : bl;
  if (length((a.tip+a.base)-(b.tip+b.base))>ra+rb)  // is the center of both nearer than radis of both *0.5 is implicit
    return false;
  if (al==0) al = 1; // a_Normal is 0 by this
  if (bl==0) bl = 1; // b_Normal is 0 by this

  Vector a_Normal = ad/al;
  Vector a_LineOffset = a_Normal * a.radius;
  Vector a_A = a.base + a_LineOffset;
  Vector a_B = a.tip - a_LineOffset;

  Vector b_Normal = bd/bl;
  Vector b_LineOffset = b_Normal * b.radius;
  Vector b_A = b.base + b_LineOffset;
  Vector b_B = b.tip - b_LineOffset;

  Vector v0 = b_A - a_A;
  Vector v1 = b_B - a_A;
  Vector v2 = b_A - a_B;
  Vector v3 = b_B - a_B;

  double d0 = dot(v0,v0);
  double d1 = dot(v1,v1);
  double d2 = dot(v2,v2);
  double d3 = dot(v3,v3);

  Vector bestA;
  if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1) {
    bestA = a_B;
  } else {
    bestA = a_A;
  }

  Vector bestB = ClosestPointOnLineSegment(b_A,b_B,bestA);
  bestA = ClosestPointOnLineSegment(a_A,a_B,bestB);

  Vector penetration_normal = bestA - bestB;
  double len = length(penetration_normal);
  penetration_normal /= len;
  double penetration_depth = a.radius+b.radius-len;
  bool intersects = penetration_depth > 0;
  return intersects;
}

bool collide(int capsuleIndexA, int capsuleIndexB) {
  return collide(capsule[capsuleIndexA],capsule[capsuleIndexB]);
}
