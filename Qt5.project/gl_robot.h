/*
 *  T3 Robot a rRRRr robot
 *
 *      Base: rotate on XY-plane
 *      Arm1: rotate w/ Z-axis
 *      Arm2: rotate w/ Z-axis
 *      Arm3: rotate w/ Z-axis
 *      Claw: rotate w/ Arm3-axis
 *
 */
#ifndef ROBOT_H
#define ROBOT_H
#include <QtOpenGL>
#include <qglviewer.h>
#include <manipulatedCameraFrame.h>
#include <qobject.h>

using namespace qglviewer;
using namespace std;

extern QString sM4x4Print(const GLdouble *m);

#define ROBOT_BONES 5
enum ROBOTS_BONE {E0_BASE, E1_ARM1, E2_ARM2, E3_ARM3, E4_CLAW, E5_NONE};

//Initial robot parameters----------------------------------------------------
const QString slBoneName[ROBOT_BONES]   = {"Base@X","Arm1@Z","Arm2@Z","Arm3@Z","Claw@X"};
const double fBoneLen[ROBOT_BONES]      = {     0.5,     1.5,     1.0,     1.0,     1.0};    //Robot bone length
const double degBoneAngleZ[ROBOT_BONES] = {       0,      60,     -30,     -30,       0};    //Robot bone angle Z-X
const double degBoneRotX[ROBOT_BONES]   = {       0,       0,       0,       0,     -90};    //Robot bone angle X-Y
const int degBoneAngleMin[ROBOT_BONES]  = {    -179,     -89,     -89,     -89,    -179};    //Angle limitation, the value are intended for easy degbugging
const int degBoneAngleMax[ROBOT_BONES]  = {     179,      89,      89,      89,     179};    //Angle limitation, the value are intended for easy degbugging

//----------------------------------------------------------------------------
//The vecTranslte[] defined each frame's translation vector
const Vec vecTranslate[ROBOT_BONES]     = { Vec(0.0, 0.0, 0),
                                            Vec(0.0, 0.0, fBoneLen[E0_BASE]),           //Arm1 h= 0+Base
                                            Vec(0.0, 0.0, fBoneLen[E1_ARM1]),           //Arm2 h= Base+Arm1
                                            Vec(0.0, 0.0, fBoneLen[E2_ARM2]),           //Arm3 h= Base+Arm1+Arm2
                                            Vec(0.0, 0.0, fBoneLen[E3_ARM3])            //Claw h= Base+Arm1+Arm2+Arm3
                                            };
//The radRotation[] defined each frame's rotation degrees in radian
const qreal radRotation[ROBOT_BONES]    = { qDegreesToRadians(  degBoneRotX[E0_BASE]),  //BaseX, to Y @-90°X
                                            qDegreesToRadians(degBoneAngleZ[E1_ARM1]),  //Arm1Z, to X @-60°Z
                                            qDegreesToRadians(degBoneAngleZ[E2_ARM2]),  //Arm2Z, to X @30°Z
                                            qDegreesToRadians(degBoneAngleZ[E3_ARM3]),  //Arm3Z, to X @30°Z
                                            qDegreesToRadians(  degBoneRotX[E4_CLAW])   //ClawX, to Y @0°X
                                            };
//The vecRotation[] defined each frame's rotation vector
const Vec  vecRotation[ROBOT_BONES]     = { Vec(0.0, 0.0, 1.0), //Z
                                            Vec(0.0, 1.0, 0.0), //Y
                                            Vec(0.0, 1.0, 0.0), //Y
                                            Vec(0.0, 1.0, 0.0), //Y
                                            Vec(0.0, 0.0, 1.0)  //Z
                                            };

//----------------------------------------------------------------------------
class GL_Robot: public QObject {
Q_OBJECT

public:
  GL_Robot();

  qglviewer::Frame *frame[ROBOT_BONES];
  int idSelected = E5_NONE; //None selected

  void setColor(unsigned short nb);
  void draw(bool names = false);

  Vec vLastPosition[ROBOT_BONES];         //Vec array for last positions
  Vec vLastRotAxis[ROBOT_BONES];          //Vec array for last rotation axis
  int iLastRotationDegrees[ROBOT_BONES];  //Last rotation degrees

public slots:
  void slotFrameUpdated();

signals:
    void sigSendBoneAngle(int boneIdx, double angleDegree);

private:

  qglviewer::AxisPlaneConstraint *constraints[3];
  unsigned short activeConstraint;

  void drawCone(float zMin, float zMax, float r1, float r2, int nbSub);
  void drawBase();
  void drawArm1();
  void drawArm2();
  void drawArm3();
  void drawJoint();
  void drawHead();
  void drawClaw();
};

#endif //ROBOT_H
