#ifndef __GAMEOBJ_HPP__
#define __GAMEOBJ_HPP__

#include "vector.hpp"

class GO {
public:
  bool deleteIt;
  Go() {
    deleteIt = false;
  }
  virtual ~GO() {
  }
};

class GO_Position {
public:
  Vector position;
  GO_Position() {
  }
  GO_Position(const Vector &position) {
    this->position = position;
  }
  virtual ~GO_Position() {
  }
};

class GO_Rotation {
public:
  Vector axis;
  double angle;
  double angleAddPerSecond;
  GO_Rotation() {
    angle = 0;
    angleAddPerSecond = 0;
  }
  virtual ~GO_Rotation() {
  }
};

class GO_State {
public:
  int state;
  GO_State() {
    state = 0;
  }
  virtual ~GO_State() {
  }
};

class GO_Callback {
public:
  GO_Callback1() {
  }
  virtual ~GO_Callback() {
  }
  virtual void callback(double dt) = 0;
};

class GO_FrequencyCallback {
public:
  double durationTillNext;
  double durationFull;
  int frequencyStep;
  GO_FrequencyCallback(double duration) {
    durationFull = duration;
    durationTillNext = duration;
    frequencyStep = 0;
  }
  virtual ~GO_FrequencyCallback() {
  }
  virtual void frequent(int iteration) = 0;
};

class GO_Physical {
public:
  Vector velocity;
  Vector force;
  GO_Physical() {
  }
  virtual ~GO_Physical() {
  }
};


class GO_LifeTime {
public:
  double lifeTime;
  GO_LifeTime(double seconds=0) {
    lifeTime = seconds;
  }
  virtual ~GO_LifeTime() {
  }
  void initLifeTime(double s) {lifeTime = s;}
};

class GO_AliveDistance {
public:
  Vector aliveLastPosition;
  bool aliveFresh;
  double aliveDistance;
  GO_AliveDistance(double d=0) {
    aliveDistance = d;
    aliveFresh = true;
  }
  virtual ~GO_AliveDistance() {
  }
  void initAliveDistance(double d) {aliveDistance = d; aliveFresh = true;}
};


class GO_Paintable {
public:
  GO_Paintable() {
  }
  virtual ~GO_Paintable() {
  }
  virtual void paint() = 0;
};

#endif // __GAMEOBJ_HPP__
