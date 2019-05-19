/****************************************************************************

 Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.7.1.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License 
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain 
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/
#include "my_utils.h"
#include "gl_robot.h"
#include <math.h>
#include <QDebug>

//using namespace qglviewer;
//using namespace std;

//////////////////////////////////  Robots  ///////////////////////////////////////////

GL_Robot::GL_Robot() {

  for (int i = 0; i < ROBOT_BONES; ++i) {// Creates a hierarchy of frames.
    frame[i] = new ManipulatedFrame();
    if (i > 0)
      frame[i]->setReferenceFrame(frame[i - 1]);

    connect(frame[i], SIGNAL(modified()), this, SLOT(slotFrameUpdated()));
    }

  // Initialize frames
  frame[E0_BASE]->setRotation(Quaternion(vecRotation[E0_BASE], radRotation[E0_BASE]));

  frame[E1_ARM1]->setRotation(Quaternion(vecRotation[E1_ARM1], radRotation[E1_ARM1]));
  frame[E1_ARM1]->setTranslation(vecTranslate[E1_ARM1]);

  frame[E2_ARM2]->setRotation(Quaternion(vecRotation[E2_ARM2], radRotation[E2_ARM2]));
  frame[E2_ARM2]->setTranslation(vecTranslate[E2_ARM2]);
  frame[E3_ARM3]->setRotation(Quaternion(vecRotation[E3_ARM3], radRotation[E3_ARM3]));
  frame[E3_ARM3]->setTranslation(vecTranslate[E3_ARM3]);

  frame[E4_CLAW]->setRotation(Quaternion(vecRotation[E4_CLAW], radRotation[E4_CLAW]));
  frame[E4_CLAW]->setTranslation(vecTranslate[E4_CLAW]);

  // Set frame constraints, World first, E0_Base constraint to Z-axis
  WorldConstraint *ZAxis = new WorldConstraint();
  ZAxis->setTranslationConstraint(AxisPlaneConstraint::PLANE, Vec(0.0, 0.0, 1.0));
  ZAxis->setRotationConstraint(AxisPlaneConstraint::AXIS, Vec(0.0, 0.0, 1.0));
  frame[E0_BASE]->setConstraint(ZAxis); //Base constraint to Z-axis

  //Arm1, Arm2, Arm3 contraint to itself on X-axis
  LocalConstraint *XAxis = new LocalConstraint();
  XAxis->setTranslationConstraint(AxisPlaneConstraint::FORBIDDEN, Vec(0.0, 0.0, 0.0));
  XAxis->setRotationConstraint(AxisPlaneConstraint::AXIS, Vec(1.0, 0.0, 0.0));
  frame[E1_ARM1]->setConstraint(XAxis); //X-axis
  frame[E2_ARM2]->setConstraint(XAxis); //X-axis
  frame[E3_ARM3]->setConstraint(XAxis); //X-axis

  frame[E4_CLAW]->setConstraint(ZAxis);
 }

/*
 * slotFrameUpdated()
 *
 *      slot to update T3 frame bones' info
 *              |        . .
 *              |       . .     /
 *              |      . +----+/[4]
 *              |     . /[3]
 *              |    . /
 *              |   +[2]
 *              |  /
 *              | /
 *              +/[1]
 *      --------+-------------------
 *              [0]
 */
void GL_Robot::slotFrameUpdated()
{
    //T3 rRRRr, Base rotates at Z-axis, angle with X-axis
    Vec v         = frame[E4_CLAW]->position();
    double angle  = qAtan2(v.y, v.x);
    double angleX = qRound(qRadiansToDegrees(angle));
    QString s;
    s.sprintf("(%+5.3f,%+5.3f,%+5.3f) @X%+4.0f°", v.x,v.y,v.z, angleX);
    qDebug().noquote() << "EndE@X" << s;

    vLastPosition[0]        = v;
    iLastRotationDegrees[0] = static_cast<int>(angleX);

    //T3 rRRRr, Arms--------------------------
    double anglePrevious = 0;
    for (int i = 1; i < ROBOT_BONES-1; ++i) {
        Vec v         = frame[i]->position() - frame[i+1]->position();
        double d      = qSqrt(v.x*v.x + v.y*v.y);
        double angle  = qAtan2(d, v.z); //atan2(distance/Z)
        if (v.x <= 0.0) { angle = M_PI  - angle;} //adjust for -PI~+PI
        else            { angle = angle - M_PI;}
        double angleZ = qRound(qRadiansToDegrees(angle));
        s.sprintf("(%+5.3f,%+5.3f,%+5.3f) @Z%+4.0f°|%+4.0f°", v.x,v.y,v.z, angleZ, angleZ-anglePrevious);
        qDebug().noquote() << slBoneName[i] << s ;

        vLastPosition[i]        = v;
        iLastRotationDegrees[i] = static_cast<int>(angleZ);
        anglePrevious           = angleZ;
        }

    //T3 rRRRr, Claw-------------------------------------------------------
    v      = frame[ROBOT_BONES-1]->position();
    angle  = qAtan2(v.y, v.x); //[QTBUG-12515]qAtan2 calls atan2/atan2f and gives the arguments in the wrong order!
    angleX = qRound(qRadiansToDegrees(angle));

    vLastPosition[ROBOT_BONES-1]        = v;
    iLastRotationDegrees[ROBOT_BONES-1] = static_cast<int>(angleX);
    s.sprintf("(%+5.3f,%+5.3f,%+5.3f) @X%+4.0f°", v.x,v.y,v.z, angleX);
    qDebug().noquote() << slBoneName[ROBOT_BONES-1] << s;
}

/*
 * ROBOT draw
 *
 */
void GL_Robot::draw(bool names) {

  // Robots' local frame
  glPushMatrix();

  //const float scale = 5.0f;
  //glScalef(scale, scale, scale);
  glEnable(GL_NORMALIZE);//To preventing lighting change after scaled!

  //--------------------------------0
  //Base
  if (names)
    glPushName(E0_BASE);
  glMultMatrixd(frame[E0_BASE]->matrix());
  setColor(E0_BASE);
  drawBase();
  if (names)
    glPopName();
  //--------------------------------1
  //Arm1
  if (names)
    glPushName(E1_ARM1);
  glMultMatrixd(frame[E1_ARM1]->matrix());
  setColor(E1_ARM1);
  drawJoint();
  drawArm1();
  if (names)
    glPopName();
  //--------------------------------2
  //Arm2
  if (names)
    glPushName(E2_ARM2);
  glMultMatrixd(frame[E2_ARM2]->matrix());
  setColor(E2_ARM2);
  drawJoint();
  drawArm2();
  if (names)
    glPopName();
  //--------------------------------3
  //Arm3
  if (names)
    glPushName(E3_ARM3);
  glMultMatrixd(frame[E3_ARM3]->matrix());
  setColor(E3_ARM3);
  drawJoint();
  drawArm3();
  if (names)
    glPopName();
  //--------------------------------4
  //Claw
  if (names)
    glPushName(E4_CLAW);
  glMultMatrixd(frame[E4_CLAW]->matrix());
  setColor(E4_CLAW);
  drawClaw();
  if (names)
    glPopName();

  glPopMatrix();
}

/*
    // Draws truncated cones aligned with the Z axis.
    drawCone(zMin, zMax, r1, r2, int nbSub)

 */
void GL_Robot::drawBase() {
  float fLen = fBoneLen[E0_BASE];
  drawCone(           0.0f, fLen* 6.0f/10.0f,  0.5f,  0.5f, 32); //base cylinder
  drawCone(fLen*6.0f/10.0f, fLen* 8.0f/10.0f,  0.5f,  0.3f, 32);
  drawCone(fLen*8.0f/10.0f, fLen*10.0f/10.0f,  0.3f,  0.2f, 32);
}

void GL_Robot::drawArm1() {
  float fLen = fBoneLen[E1_ARM1];
  glTranslatef(0.1f, 0.0, 0.0);
  //drawCone(0.0, 0.5f, 0.01f, 0.01f, 10);
  drawCone(0.0, fLen, 0.18f, 0.15f, 8); //arm-left
  glTranslatef(-0.2f, 0.0, 0.0);
  //drawCone(0.0, 0.5f, 0.01f, 0.01f, 10);
  drawCone(0.0, fLen, 0.18f, 0.15f, 8); //arm-right
  glTranslatef(0.1f, 0.0, 0.0);
}
void GL_Robot::drawArm2() {
  float fLen = fBoneLen[E2_ARM2];
  glTranslatef(0.1f, 0.0, 0.0);
  //drawCone(0.0, 0.5f, 0.01f, 0.01f, 10);
  drawCone(0.0, fLen, 0.18f, 0.1f, 4); //arm-left
  glTranslatef(-0.2f, 0.0, 0.0);
  //drawCone(0.0, 0.5f, 0.01f, 0.01f, 10);
  drawCone(0.0, fLen, 0.18f, 0.1f, 4); //arm-right
  glTranslatef(0.1f, 0.0, 0.0);
}
void GL_Robot::drawArm3() {
  float fLen = fBoneLen[E3_ARM3];
  glTranslatef(0.1f, 0.0, 0.0);
  //drawCone(0.0, 0.5f, 0.01f, 0.01f, 10);
  drawCone(0.0, fLen, 0.18f, 0.1f, 4); //arm-left
  glTranslatef(-0.2f, 0.0, 0.0);
  //drawCone(0.0, 0.5f, 0.01f, 0.01f, 10);
  drawCone(0.0, fLen, 0.18f, 0.1f, 4); //arm-right
  glTranslatef(0.1f, 0.0, 0.0);
}
void GL_Robot::drawHead() {
  drawCone(-0.125f,-0.1f, 0.05f, 0.2f, 30);
  drawCone(-0.10f,  0.3f, 0.2f, 0.2f, 30);
  drawCone( 0.3f,  0.75f, 0.2f, 0.85f, 30);
  drawCone( 0.85f,  0.85f, 0.85f, 0.85f, 30);
}
void GL_Robot::drawClaw() {
  float fLen = fBoneLen[E4_CLAW];

  drawCone(-0.125f,-0.1f, 0.05f, 0.25f, 10);
  drawCone(-0.1f,  0.15f, 0.15f, 0.15f, 10);
  drawCone( 0.15f,  0.15f, 0.05f, 0.15f, 10);

  glRotatef(-90, 1,0,0); //rotate 180deg on X
  glTranslatef(0.1f, 0.0, 0.0);
  drawCone(0.0, fLen, 0.1f, 0.05f, 3); //arm-left
  glTranslatef(-0.2f, 0.0, 0.0);
  drawCone(0.0, fLen, 0.1f, 0.05f, 3); //arm-right
  glTranslatef(0.1f, 0.0, 0.0);

}
void GL_Robot::drawJoint() {
  glPushMatrix();
  //glRotatef(90, 0.0, 1.0, 0.0);                  //Yaxis @90degree
  glRotatef(-90, 1.0, 0.0, 0.0);                  //-Yaxis @90degree

  float fWidth = 0.25f; //cylinder width
  drawCone(-0.16f, -0.15f,   0.0f,  fWidth, 32);  //left side disk
  drawCone(-0.15f,  0.15f, fWidth,  fWidth, 32);  //cylinder
  drawCone( 0.15f,  0.16f,   0.0f,  fWidth, 32);  //right side disk
  glPopMatrix();
}

void GL_Robot::setColor(unsigned short nb) {
  if (nb == idSelected)
    glColor3f(212.0f/255.0f, 175.0f/255.0f, 55.0f/255.0f); //Gold
  else
    glColor3f(69.0f/255.0f, 142.0f/255.0f, 97.0f/255.0f); //SeaGreen(Amazon)
    //glColor3f(29.0f/255.0f,  92.0f/255.0f, 57.0f/255.0f); //SeaGreen(Amazon)
}

// Draws a truncated cone aligned with the Z axis.
void GL_Robot::drawCone(float zMin, float zMax, float r1, float r2, int nbSub) {
  static GLUquadric *quadric = gluNewQuadric();

  glTranslatef(0.0, 0.0, zMin);
  gluCylinder(quadric, r1, r2, zMax - zMin, nbSub, 1);
  glTranslatef(0.0, 0.0, -zMin);
}
