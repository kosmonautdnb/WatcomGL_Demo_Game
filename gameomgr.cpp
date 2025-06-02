#include "gameobj.hpp"
#include "gameomgr.hpp"
#include "colision.hpp"

Array<GO*> gameObjects;
Array<GO_Manager*> goManagers;

extern double levelScrollY;


bool isInActiveScreen(const Vector &position) {
  const double y = position.y + levelScrollY;
  const double x = position.x;
  if (y > (-65.0-30) && y < (65.0+30)) {
    if (x > (-100.0-50) && x < (100.0+50)) 
      return true;
  }
  return false;
}

bool GO_GOManager::addObject(GO *o) {
  managed.push_back(o);
  return true;
}

void GO_GOManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if ((!o->activated)&&(!o->deleteIt)) {
      GO_Position *v0 = dynamic_cast<GO_Position*>(o);
      if (v0 != NULL) {
        if (isInActiveScreen(v0->position)) o->activated = true;
      }
    }
    if ((o->activated)&&(!o->deleteIt)) {
      GO_Position *v1 = dynamic_cast<GO_Position*>(o);
      if (v1 != NULL) {
        if (!isInActiveScreen(v1->position)) o->deleteIt = true;
      }
    }
    if (o->deleteIt||(!o->activated)) continue;
    o->seconds += dt;
  }
}

bool GO_CallbackManager::addObject(GO *o) {
  if (dynamic_cast<GO_Callback*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_CallbackManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_Callback *v0 = dynamic_cast<GO_Callback*>(o);
    v0->callback(dt);
  }
}

bool GO_FrequencyCallbackManager::addObject(GO *o) {
  if (dynamic_cast<GO_FrequencyCallback*>(o)==NULL&&dynamic_cast<GO_FrequencyCallback2*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_FrequencyCallbackManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;

    GO_FrequencyCallback *v0 = dynamic_cast<GO_FrequencyCallback*>(o);
    if (v0 != NULL) {
      v0->durationTillNext -= dt;
      if (v0->durationTillNext <= 0) {
        v0->frequent(v0->frequencyStep);
        v0->frequencyStep++;
        v0->durationTillNext = v0->durationFull;
      }
    }

    GO_FrequencyCallback2 *v1 = dynamic_cast<GO_FrequencyCallback2*>(o);
    if (v1 != NULL) {
      v1->durationTillNext2 -= dt;
      if (v1->durationTillNext2 <= 0) {
        v1->frequent2(v1->frequencyStep2);
        v1->frequencyStep2++;
        v1->durationTillNext2 = v1->durationFull2;
      }
    }
  }
}

bool GO_RotationManager::addObject(GO *o) {
  if (dynamic_cast<GO_Rotation*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_RotationManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_Rotation *v0 = dynamic_cast<GO_Rotation*>(o);
    v0->angle += v0->angleAddPerSecond * dt;
  }
}

bool GO_SimplePhysicsManager::addObject(GO *o) {
  if (dynamic_cast<GO_Position*>(o)==NULL) return false;
  if (dynamic_cast<GO_Physical*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_SimplePhysicsManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_Position *v0 = dynamic_cast<GO_Position*>(o);
    GO_Physical *v1 = dynamic_cast<GO_Physical*>(o);
    v1->velocity += v1->force * dt;
    v1->force.zero();
    v0->position += v1->velocity * dt;
  }
}

bool GO_LifeTimeManager::addObject(GO *o) {
  if (dynamic_cast<GO_LifeTime*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_LifeTimeManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_LifeTime *v0 = dynamic_cast<GO_LifeTime*>(o);
    v0->lifeTime -= dt;
    if (v0->lifeTime < 0) {o->destruct();}
  }
}

bool GO_AliveDistanceManager::addObject(GO *o) {
  if (dynamic_cast<GO_AliveDistance*>(o)==NULL) return false;
  if (dynamic_cast<GO_Position*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_AliveDistanceManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_Position *v0 = dynamic_cast<GO_Position*>(o);
    GO_AliveDistance *v1 = dynamic_cast<GO_AliveDistance*>(o);
    Vector lastPosition = v1->aliveLastPosition;
    if (v1->aliveFresh) {lastPosition = v0->position;v1->aliveFresh=false;}
    double d = length(v0->position-lastPosition);
    v1->aliveLastPosition = v0->position;
    v1->aliveDistance -= d;
    if (v1->aliveDistance < 0) {o->destruct();}
  }
}

bool GO_PaintableManager::addObject(GO *o) {
  if (dynamic_cast<GO_Paintable*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_PaintableManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_Paintable *v0 = dynamic_cast<GO_Paintable*>(o);
    v0->paint(dt);
  }
}

bool GO_Collider_Enemy_Manager::addObject(GO *o) {
  if (dynamic_cast<GO_Collider_Enemy*>(o)==NULL) return false;
  if (dynamic_cast<GO_Position*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void playerHit(bool explosion);

void GO_Collider_Enemy_Manager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_Position *v0 = dynamic_cast<GO_Position*>(o);
    GO_Collider_Enemy *v1 = dynamic_cast<GO_Collider_Enemy*>(o);
    v1->lastColliderEnemyPosition = v1->colliderEnemyPosition;
    v1->colliderEnemyPosition = v0->position;
    if (v1->colliderEnemyFresh) {v1->colliderEnemyFresh--;continue;}
    capsule[CAPSULE_COLLIDER] = Capsule(v1->colliderEnemyPosition, v1->lastColliderEnemyPosition, v1->colliderEnemyRadius);
    if (collide(CAPSULE_COLLIDER,CAPSULE_PLAYER)) {
      playerHit(true);
    }
  }
}

bool GO_Collider_LevelObject_Manager::addObject(GO *o) {
  if (dynamic_cast<GO_Collider_LevelObject*>(o)==NULL) return false;
  if (dynamic_cast<GO_Position*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_Collider_LevelObject_Manager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt||(!o->activated)) continue;
    GO_Position *v0 = dynamic_cast<GO_Position*>(o);
    GO_Collider_LevelObject *v1 = dynamic_cast<GO_Collider_LevelObject*>(o);
    v1->lastColliderLevelObjectPosition = v1->colliderLevelObjectPosition;
    v1->colliderLevelObjectPosition = v0->position;
    if (v1->colliderLevelObjectFresh) {v1->colliderLevelObjectFresh--;continue;}
    capsule[CAPSULE_COLLIDER] = Capsule(v1->colliderLevelObjectPosition, v1->lastColliderLevelObjectPosition, v1->colliderLevelObjectRadius);
    if (collide(CAPSULE_COLLIDER,CAPSULE_PLAYER)) {
      playerHit(true);
    }
  }
}

void go_clearManagers() {
  for (int i = 0; i < goManagers.size(); i++)
    delete goManagers[i];
  goManagers.clear();
}

void go_setupManagers() {
  go_clearManagers();
  goManagers.push_back(new GO_GOManager());
  goManagers.push_back(new GO_CallbackManager());
  goManagers.push_back(new GO_RotationManager());
  goManagers.push_back(new GO_SimplePhysicsManager());
  goManagers.push_back(new GO_LifeTimeManager());
  goManagers.push_back(new GO_AliveDistanceManager());
  goManagers.push_back(new GO_PaintableManager());
  goManagers.push_back(new GO_FrequencyCallbackManager());
  goManagers.push_back(new GO_Collider_Enemy_Manager());
  goManagers.push_back(new GO_Collider_LevelObject_Manager());
}


void go_setup(GO_Manager *m, double dt) {
  m->clear();
  m->dt = dt;
}

void go_onFrame(double dt) {
  // collect all into the appropiate managers
  for (int i = 0; i < goManagers.size(); i++) {
    go_setup(goManagers[i],dt);
    for (int j = 0; j < gameObjects.size(); j++) {
      goManagers[i]->addObject(go_(gameObjects[j]));
    }
  }

  // handle all objects
  for (int k = 0; k < goManagers.size(); k++) {
    goManagers[k]->manage();
  }

  // delete all game objects that should be deleted
  for (int j = 0; j < gameObjects.size(); j++) {
    if (gameObjects[j]->deleteIt) {
     delete go_(gameObjects[j]);
     gameObjects.erase(j,1);
     j--;
    }
  }
}
