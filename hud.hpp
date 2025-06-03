void hudStart() {
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,1280,720,0,-1,1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
}

void hudEnd() {
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

void drawEnergyBar(double xp, double yp, double w, double h) {
  double shorten=(double)playerHits/maxPlayerHits;
  float r2 = shorten;
  float g2 = 1-saturate(shorten+0.5);
  float b2 = 0;
  float r1 = shorten;
  float g1 = 1*(1-shorten);
  float b1 = 0;  
  double aw = w;
  w *= 1-shorten;
  xp += aw-w;
  float k = 5.0;
  glBegin(GL_QUADS);
  glColor4f(1,1,1,1);
  glColor3f(r2*k,g2*k,b2*k);
  glVertex3f(xp+w,yp,0);
  glColor3f(r1*k,g1*k,b1*k);
  glVertex3f(xp,yp,0);
  glColor3f(r1,g1,b1);
  glVertex3f(xp,yp+h,0);
  glColor3f(r2,g2,b2);
  glVertex3f(xp+w,yp+h,0);
  glEnd();
}

void drawHudGetReady(double anim) {
  hudStart();
  drawText(1280/2,720/2,"Get Ready",0x00ffffff+((uint32_t)(pow(anim,0.5)*0xff000000)&0xff000000),1.0,0.5,0.5);
  hudEnd();
}

void drawHud() {
  hudStart();

  uint32_t color = 0xffffffc0;
  char scoreBuffer[16];
  sprintf(scoreBuffer,"%08d",score);
  drawText(0,0,scoreBuffer,color);
  sprintf(scoreBuffer,"%02d",lives);
  drawText(1280,0,scoreBuffer,color,1,1,0);

  sprintf(scoreBuffer,"fps:%d",(int)fps);
  drawText(0,40,scoreBuffer,color,1.0);
  int debugCount = 0;
  for (int i = 0; i < gameObjects.size(); i++) {
    if (gameObjects[i]->debugCountIt)
      debugCount++;
  }
  //sprintf(scoreBuffer,"%d/%d",(int)gameObjects.size(),debugCount);
  //drawText(0,80,scoreBuffer,color,1.0);


  drawEnergyBar(1280-250,5,150,20);

  hudEnd();
}
