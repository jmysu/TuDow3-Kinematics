#ifndef GL_PARTICLE_H
#define GL_PARTICLE_H
#include <QtOpenGL>
#include <qglviewer.h>
//#include <manipulatedCameraFrame.h>
#include <math.h>
#include <stdlib.h> // RAND_MAX

using namespace qglviewer;
using namespace std;

class Particle {
public:
  Particle();

  void init();
  void draw();
  void animate();
  void setPosition(qreal x, qreal y, qreal z);

private:
  qglviewer::Vec speed, pos, pos0;
  int age, ageMax;
};
#endif // GL_PARTICLE_H
