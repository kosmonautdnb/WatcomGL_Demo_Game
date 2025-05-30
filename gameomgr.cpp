#include "gameobj.hpp"
#include "gameomgr.hpp"
#include "colision.hpp"

Array<GO*> gameObjects;
Array<GO_Manager*> goManagers;

bool GO_CallbackManager::addObject(GO *o) {
  if (dynamic_cast<GO_Callback*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_CallbackManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt) continue;
    GO_Callback *v0 = dynamic_cast<GO_Callback*>(o);
    v0->callback(dt);
  }
}

bool GO_FrequencyCallbackManager::addObject(GO *o) {
  if (dynamic_cast<GO_FrequencyCallback*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void GO_FrequencyCallbackManager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt) continue;
    GO_FrequencyCallback *v0 = dynamic_cast<GO_FrequencyCallback*>(o);
    v0->durationTillNext -= dt;
    if (v0->durationTillNext <= 0) {
      v0->frequent(v0->frequencyStep);
      v0->frequencyStep++;
      v0->durationTillNext = v0->durationFull;
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
    if (o->deleteIt) continue;
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
    if (o->deleteIt) continue;
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
    if (o->deleteIt) continue;
    GO_LifeTime *v0 = dynamic_cast<GO_LifeTime*>(o);
    v0->lifeTime -= dt;
    if (v0->lifeTime < 0) o->deleteIt = true;
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
    if (o->deleteIt) continue;
    GO_Position *v0 = dynamic_cast<GO_Position*>(o);
    GO_AliveDistance *v1 = dynamic_cast<GO_AliveDistance*>(o);
    Vector lastPosition = v1->aliveLastPosition;
    if (v1->aliveFresh) {lastPosition = v0->position;v1->aliveFresh=false;}
    double d = length(v0->position-lastPosition);
    v1->aliveLastPosition = v0->position;
    v1->aliveDistance -= d;
    if (v1->aliveDistance < 0) o->deleteIt = true;
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
    if (o->deleteIt) continue;
    GO_Paintable *v0 = dynamic_cast<GO_Paintable*>(o);
    v0->paint();
  }
}

bool GO_Collider_Enemy_Manager::addObject(GO *o) {
  if (dynamic_cast<GO_Collider_Enemy*>(o)==NULL) return false;
  if (dynamic_cast<GO_Position*>(o)==NULL) return false;
  managed.push_back(o);
  return true;
}

void playerHit();

void GO_Collider_Enemy_Manager::manage() {
  for (int i = 0; i < managed.size(); i++) {
    GO *o = managed[i];
    if (o->deleteIt) continue;
    GO_Position *v0 = dynamic_cast<GO_Position*>(o);
    GO_Collider_Enemy *v1 = dynamic_cast<GO_Collider_Enemy*>(o);
    v1->lastColliderEnemyPosition = v1->colliderEnemyPosition;
    v1->colliderEnemyPosition = v0->position;
    if (v1->colliderEnemyFresh) {v1->colliderEnemyFresh=false;continue;}
    capsule[CAPSULE_COLLIDER] = Capsule(v1->colliderEnemyPosition, v1->lastColliderEnemyPosition, v1->colliderEnemyRadius);
    if (collide(CAPSULE_COLLIDER,CAPSULE_PLAYER)) {
      playerHit();
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
  goManagers.push_back(new GO_CallbackManager());
  goManagers.push_back(new GO_RotationManager());
  goManagers.push_back(new GO_SimplePhysicsManager());
  goManagers.push_back(new GO_LifeTimeManager());
  goManagers.push_back(new GO_AliveDistanceManager());
  goManagers.push_back(new GO_PaintableManager());
  goManagers.push_back(new GO_FrequencyCallbackManager());
  goManagers.push_back(new GO_Collider_Enemy_Manager());
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
