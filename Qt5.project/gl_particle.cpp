#include "gl_particle.h"


Particle::Particle( ) {
    init();
    }

/*
 * Particle init with new position (pos0)
 *
 */
void Particle::init() {
  //pos = Vec(2,2,1);
  pos = pos0;

  float angle = 3.0 * M_PI * rand() / RAND_MAX;
  float norm  = 0.4  * rand() / RAND_MAX;
  speed = Vec(norm * cos(angle), norm * sin(angle),  rand() / static_cast<float>(RAND_MAX));
  age = 0;
  ageMax = 3.0 + static_cast<int>(12.0 * rand() / RAND_MAX);
}

void Particle::setPosition(qreal x, qreal y, qreal z)
{
    pos0.x = x;
    pos0.y = y;
    pos0.z = z;
    init();
}

void Particle::animate() {
  speed.z += 0.1f;
  pos += 0.1f * speed;
  if (++age == ageMax) init();

}

void Particle::draw() {
  //Change color by age
  float fA = age/(float)ageMax;
  glColor3f(1.0 , fA , fA);
  //Change point size by age
  if (fA > 0.85f)
      glPointSize(1.5f);
  else if (fA > 0.35f)
      glPointSize(3.0f);
  else
      glPointSize(8.0f);
  //Draw point
  glBegin(GL_POINTS);
    glVertex3fv(pos);
  glEnd();
}
