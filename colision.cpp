#include "colision.hpp"
#include "util.hpp"

Capsule capsule[MAX_CAPSULES];

// wickedengine.net
Vector ClosestPointOnLineSegment(const Vector &A, const Vector &B, const Vector &Point) {
  Vector AB = B - A;
  double k = dot(AB,AB);
  if (k == 0) k = 1;
  double t = dot(Point-A,AB) / k;
  return A + saturate(t) * AB;
}

Vector collisionCenter;
Vector collisionPositionA;
Vector collisionPositionB;

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
  if (length((a.tip+a.base)-(b.tip+b.base))*0.5>ra+rb)
    return false;

  if (al == 0) al = 1;
  if (bl == 0) bl = 1;

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
  if (len > 0) penetration_normal /= len;
  double penetration_depth = a.radius+b.radius-len;
  bool intersects = penetration_depth > 0;
  if (intersects) {
    collisionCenter = (bestA+bestB)*0.5;
    collisionPositionA = bestA;
    collisionPositionB = bestB;
  }
  return intersects;
}

bool collide(int capsuleIndexA, int capsuleIndexB) {
  return collide(capsule[capsuleIndexA],capsule[capsuleIndexB]);
}
