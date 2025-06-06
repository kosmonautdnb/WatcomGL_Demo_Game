#ifndef __GAMEOMGR_HPP__
#define __GAMEOMGR_HPP__

#include "array.hpp"
#include "gameobj.hpp"

#define __RUNNING ((o->active)&&(!o->deleteIt))

class GO_Manager {
public:
  GO_Manager() {
    dt = 0;
  }
  virtual ~GO_Manager() {
  }
  Array<GO*> managed;
  double dt;
  virtual void clear() {
    managed.clear();
  }
  virtual bool addObject(GO *o)=0;
  virtual void manage()=0;
};                                             

class GO_GOManager : public GO_Manager {
public:
  GO_GOManager() : GO_Manager() {
  }
  virtual ~GO_GOManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_CallbackManager : public GO_Manager {
public:
  GO_CallbackManager() : GO_Manager() {
  }
  virtual ~GO_CallbackManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_FrequencyCallbackManager : public GO_Manager {
public:
  GO_FrequencyCallbackManager() : GO_Manager() {
  }
  virtual ~GO_FrequencyCallbackManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_RotationManager : public GO_Manager {
public:
  GO_RotationManager() : GO_Manager() {
  }
  virtual ~GO_RotationManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_SimplePhysicsManager : public GO_Manager {
public:
  GO_SimplePhysicsManager() : GO_Manager() {
  }
  virtual ~GO_SimplePhysicsManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_LifeTimeManager : public GO_Manager {
public:
  GO_LifeTimeManager() : GO_Manager() {
  }
  virtual ~GO_LifeTimeManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_AliveDistanceManager : public GO_Manager {
public:
  GO_AliveDistanceManager() : GO_Manager() {
  }
  virtual ~GO_AliveDistanceManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_PaintableManager : public GO_Manager {
public:
  GO_PaintableManager() : GO_Manager() {
  }
  virtual ~GO_PaintableManager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_Collider_Enemy_Manager : public GO_Manager {
public:
  GO_Collider_Enemy_Manager() : GO_Manager() {
  }
  virtual ~GO_Collider_Enemy_Manager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

class GO_Collider_LevelObject_Manager : public GO_Manager {
public:
  GO_Collider_LevelObject_Manager() : GO_Manager() {
  }
  virtual ~GO_Collider_LevelObject_Manager() {
  }
  virtual bool addObject(GO *o);
  virtual void manage();
};

extern Array<class GO*> gameObjects;
extern Array<class GO_Manager*> goManagers;

void go_clearManagers();
void go_setupManagers();
void go_onFrame(double dt);

#endif // __GAMEOMGR_HPP__
