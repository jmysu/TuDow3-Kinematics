/*
 * gl_viewer.cpp
 *
 *  see https://github.com/jmysu/TuDow3-Kinematics
 *
 *
 */
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


#include <math.h>
#include <QDebug>
#include <qglviewer.h>
#include <manipulatedCameraFrame.h>

#include "gl_viewer.h"

//using namespace qglviewer;
//using namespace std;

//////////////////////////////////  V i e w e r  ///////////////////////////////////////////

void Viewer::init() {
  restoreStateFromFile();

  //Make camera the default manipulated frame without Modifier key
  //setManipulatedFrame(camera()->frame());
  setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::CAMERA,  QGLViewer::ROTATE);
  setMouseBinding(Qt::NoModifier, Qt::RightButton, QGLViewer::CAMERA, QGLViewer::TRANSLATE);
  setWheelBinding(Qt::AltModifier, QGLViewer::CAMERA, QGLViewer::ZOOM);

  //Make frameParticle the manipulated frame with Shift Modifier key
  setManipulatedFrame( &frameEndEffector);
  //setMouseBinding(Qt::NoModifier, Qt::LeftButton, QGLViewer::FRAME,   QGLViewer::ROTATE);
  setMouseBinding(Qt::ShiftModifier, Qt::LeftButton, QGLViewer::FRAME,  QGLViewer::TRANSLATE);
  setWheelBinding(Qt::ShiftModifier, QGLViewer::FRAME, QGLViewer::ZOOM);

  initSpotLight();

  //Create particles
  //frameParticle.setPosition(3,3,0); //set initial frameParticle position
  frameEndEffector.setPosition(myRobot.frame[E4_CLAW]->position()); //
  for (int i = 0; i < iParticleParts; ++i) {
    particle[i] = new Particle();     
    }
  startAnimation();

  //updateBoneList();
 }

void Viewer::initSpotLight() {
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_LIGHT1);
    glLoadIdentity();

    // Light default parameters
    GLfloat pos[4]              = {1.0f, -1.0f, 1.0f, 1.0f};
    GLfloat spot_dir[3]         = {-1.0f, 1.0f, -1.0f};
    GLfloat light_ambient[4]    = {0.1f, 0.3f, 0.1f, 1.0f};
    GLfloat light_specular[4]   = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat light_diffuse[4]    = {0.5f, 0.7f, 0.5f, 1.0f};

    glLightfv(GL_LIGHT1, GL_POSITION, pos);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_dir);
    glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 3.0);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 50.0);
    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 0.5);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 1.0);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 1.5);
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
}

void Viewer::addInfoLeft(QString s)
{
    for (int i=1;i<iInfoLines;i++) sInfoLeft[i-1] = sInfoLeft[i];
    sInfoLeft[iInfoLines-1] = s;
}
void Viewer::addInfoLeft(int row, QString s)
{
    sInfoLeft[row] = s;
}
void Viewer::clearInfoLeft()
{
    for (int i=0;i<iInfoLines;i++) sInfoLeft[i]="";
}

void Viewer::addInfoRight(QString s)
{
    for (int i=1;i<iInfoLines;i++) sInfoRight[i-1] = sInfoRight[i];
    sInfoRight[iInfoLines-1] = s;
}
void Viewer::addInfoRight(int row, QString s)
{
    sInfoRight[row] = s;
}
void Viewer::clearInfoRight()
{
    for (int i=0;i<iInfoLines;i++) sInfoRight[i]="";
}

void Viewer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
        case Qt::Key_I : bShowInfo=!bShowInfo; break;

        // Default calls the original method to handle standard keys
        default: QGLViewer::keyPressEvent(e);
        }
}

void Viewer::draw() {
    if (bShowFire)
        showFireParticles();    //Show firebase and particles
    if (bShowFABRIKvec) {       //FABRIK, show arrows instead of Robot's body
        showRobotVecsArrows();
        showFABRIKvecs();
        }
    else
        myRobot.draw();
    // show Info-------------------------------------
    if (bShowInfo) {
        //Save OpenGL state
        glPushAttrib(GL_ALL_ATTRIB_BITS);
         glColor3f(0.8f, 0.8f, 0.3f);
         for (int r=0;r<iInfoLines;r++) {
            if (!sInfoLeft[r].isEmpty())
                drawText(10, 16*r, sInfoLeft[r]);
            if (!sInfoRight[r].isEmpty())
                drawText((this->width()/2 +32), 16*r, sInfoRight[r]);
            }
        // Restore GL state
        glPopAttrib();
        }
    // show dotted lines, shadow on XY plane----------
    showProjectedXY();

    // add some light--------------------------------
    const GLfloat pos[4] = {1.0, -1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT1, GL_POSITION, pos);
    const GLfloat spot_dir[3] = {-1.0, 1.0, -1.0};
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_dir);

    /*
    //debug
    glPushAttrib(GL_ALL_ATTRIB_BITS);
     glColor3f(1.0f, 0.5f, 0.5f);
     for (int r=0; r<ROBOT_BONES;r++) {
        QString s;
        s.sprintf("Axis:%d", r);
        drawText(10, 16*(r+1) , s + sV3print(myRobot.lastRotAxis[r]));
        }
    glPopAttrib();*/

    if (bShowTD3)
        showTD3Mark();
    if (bShowILoveYou)
        showILoveYou();
}

void Viewer::drawWithNames() {
  // Render scene with objects ids
  //myRobot.draw(true);
}

void Viewer::animate() {
    for (int i = 0; i < iParticleParts; i++)
      particle[i]->animate();
}


void Viewer::postSelection(const QPoint &point) {
  // Find the selectedPoint coordinates, using camera()->pointUnderPixel().
  bool found;
  selectedPoint = camera()->pointUnderPixel(point, found);
  }


QString Viewer::helpString() const {
  QString sTitle= APP_NAME+QString(" V")+APP_VERSION;
  QString text("<h2>" +sTitle+ "</h2>");
  text += "Use Keyboard <i>I</i> to Enable/Disable showing the<br>";
  text += "target/teapot positions! <br><br>";
  text += "Press <b>Return</b> to start/stop the animation.";
  return text;
}

void Viewer::setColorGold(bool bGold) {
  if (bGold)
    glColor3f(212.0f/255.0f, 175.0f/255.0f, 55.0f/255.0f); //Gold
  else
    glColor3f(69.0f/255.0f, 102.0f/255.0f, 97.0f/255.0f); //DGreen
}

// Draws a truncated cone aligned with the Z axis.
void Viewer::drawCone(float zMin, float zMax, float r1, float r2, int nbSub) {
  static GLUquadric *quadric = gluNewQuadric();

  glTranslatef(0.0, 0.0, zMin);
  gluCylinder(quadric, r1, r2, zMax - zMin, nbSub, 1);
  glTranslatef(0.0, 0.0, -zMin);
}

//----------------------------------------------------------------------------
////////////////////////////////// stuff for drawing /////////////////////////

void Viewer::showRobotVecsArrows()
{
    //Show arrows instead robot bones
    if (bFABRIKok)
        glColor3f(0.3f, 0.8f, 0.3f);
    else
        glColor3f(0.3f, 0.5f, 0.3f);

    for (int i=1; i<ROBOT_BONES; i++) {
        Vec vFrom=myRobot.frame[i-1]->position();
        Vec vTo=myRobot.frame[i]->position();
        drawArrow(vFrom, vTo, 0.05);
        }
    //Show axis arrows
    //glColor3f(0.8f, 0.8f, 0.3f);
    //for (int i=0; i<ROBOT_BONES-1; i++) {
    //    Vec vFrom=myRobot.frame[i]->position();
    //    Vec vTo=vFrom+myRobot.frame[i]->rotation().axis();
    //    drawArrow(vFrom, vTo, 0.03);
    //    }

    //Show arrow pointing to fire on XY-plane
    Vec vEnd = frameEndEffector.position();
    glColor3f(1.0f, 0.5f, 0.5f);
    drawArrow(Vec(0,0,0), Vec(vEnd.x, vEnd.y, 0), 0.025, 4);
}

void Viewer::showProjectedXY()
{
    //Draw projected robot arm lines
    //---------------------------
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_LIGHTING);
      //Draw Projection lines
      Vec v2, v3, v4;
      v2 = myRobot.frame[E2_ARM2]->position();
      v3 = myRobot.frame[E3_ARM3]->position();
      v4 = myRobot.frame[E4_CLAW]->position();

      glLineWidth(2);
      glBegin(GL_LINES);
       glColor3f(0.75f, 0.35f, 0.35f);
       glVertex2d(0, 0);
       glVertex2d(v2.x, v2.y);

       glColor3f(0.35f, 0.75f, 0.35f);
       glVertex2d(v2.x, v2.y);
       glVertex2d(v3.x, v3.y);

       glColor3f(0.35f, 0.35f, 0.75f);
       glVertex2d(v3.x, v3.y);
       glVertex2d(v4.x, v4.y);
      glEnd();

      //Draw EndEffector shadow
      Vec v = v4;
      //Draw Quad
      glColor3f(0.35f, 0.35f, 0.35f);
      glBegin(GL_QUADS);
       glVertex2d(v.x-0.25, v.y-0.25);
       glVertex2d(v.x+0.25, v.y-0.25);
       glVertex2d(v.x+0.25, v.y+0.25);
       glVertex2d(v.x-0.25, v.y+0.25);
       glVertex2d(v.x-0.25, v.y-0.25);
      glEnd();
      //Draw Crosshair
      glColor3f(0.75f, 0.75f, 0.25f);
      glLineWidth(4);
      glBegin(GL_LINES);
       glVertex2d(v.x-0.25, v.y-0.25);
       glVertex2d(v.x+0.25, v.y+0.25);
       glVertex2d(v.x-0.25, v.y+0.25);
       glVertex2d(v.x+0.25, v.y-0.25);
      glEnd();
      //dotted crosslines (-5,5)
      glLineStipple(1, 0xA0A0);
      glEnable(GL_LINE_STIPPLE);
      glLineWidth(2);
      glBegin(GL_LINES);
       glVertex2d( -5, v.y);
       glVertex2d(  5, v.y);
       glVertex2d(v.x, -5);
       glVertex2d(v.x,  5);
      glEnd();
      glPopAttrib();
}

void Viewer::showFireParticles()
{
    if (bShowFire) {
       glPushMatrix();
        //const float scale = 5.0f;  //ScaleX5
        //glScalef(scale, scale, scale);
        glMultMatrixd(frameEndEffector.matrix());
        glColor3f(0.8f, 0.4f, 0.4f);
        drawFirePotBase();
       glPopMatrix();
       //Particle-----------------
       glPushMatrix();
        glMultMatrixd(frameEndEffector.matrix());
        glDisable(GL_LIGHTING);
        glBegin(GL_POINTS);
         for (int i = 0; i < iParticleParts; i++) {
            particle[i]->draw();
            }
        glEnd();
        glEnable(GL_LIGHTING);
       glPopMatrix();
    }
}

/*
 * showTD3Mark
 *
 *      baseline at (-50,0), translate to glVertex3f(x, -4, -y)
 */
void Viewer::showTD3Mark()
{
    //Rendering Chinese font--------------------------------------------------------------
    QPainterPath path;
    glColor3f(0.5f, 0.5f, 0.3f);
    path.addText(QPointF(-50, 0), QFont("WeibeiTC-Bold", 8, 87), QString("土豆3號!"));
    QList<QPolygonF> poly = path.toSubpathPolygons();

    glDisable(GL_LIGHTING);
    for (QList<QPolygonF>::iterator i = poly.begin(); i != poly.end(); i++){
        glBegin(GL_LINE_LOOP);
        for (QPolygonF::iterator p = (*i).begin(); p != i->end(); p++)
            glVertex3f(p->rx()*0.1f, -3, -p->ry()*0.1f);
        glEnd();
    }
    glEnable(GL_LIGHTING);
}

/*
 * showILoveYout()
 *
 *  base line at (-30,3), translate to (x, -1-y, 3.0)
 */
void Viewer::showILoveYou()
{
    QPainterPath *path = new QPainterPath;
    path->addText(QPointF(-25, 3), QFont("Canonatia", 32, 1), QString("I love you"));
    //path->addText(QPointF(-32, 3), QFont("WeibeiTC-Bold", 22, 50), QString("發大財"));
    path->simplified();

    QPainterPathStroker stroker;
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setWidth(1.85);
    stroker.setJoinStyle(Qt::RoundJoin); // and other adjustments you need
    newPath = (stroker.createStroke(*path) + *path).simplified();

    polyILoveYou = newPath.toSubpathPolygons();
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (QList<QPolygonF>::iterator i = polyILoveYou.begin(); i != polyILoveYou.end(); i++){
        glBegin(GL_LINE_LOOP);
        for (QPolygonF::iterator p = i->begin(); p != i->end(); p++)
            glVertex3f(p->rx()*0.1f, -0.5f-p->ry()*0.12f, 3.0f);
        glEnd();
        }
    glEnable(GL_LIGHTING);
}

void Viewer::drawFirePotBase() {
  float fLen = 0.1;
  drawCone(      0.0, fLen* 7/10,  0.2f,  0.2f, 32); //base cylinder
  drawCone(fLen*7/10, fLen* 9/10,  0.2f, 0.15f, 32);
  drawCone(fLen*9/10, fLen*10/10, 0.15f,  0.1f, 32);
}

//----------------------------------------------------------------------------
//////////////////////////////////  Mouse  ///////////////////////////////////
void Viewer::mousePressEvent(QMouseEvent* e)
{
    if ((e->button() == Qt::LeftButton) && (e->modifiers() == Qt::ShiftModifier))
        myMouseBehavior = true;
    QGLViewer::mousePressEvent(e);
}
void Viewer::mouseMoveEvent(QMouseEvent *e)
{
    if (myMouseBehavior) {
        // Use e->x() and e->y() as you want...
        //qDebug() << e->x() << e->y();
        vFABRIKtarget   = frameEndEffector.position(); //save the EE position for FABRIK processing
        double rTargetX =  atan2(vFABRIKtarget.y, vFABRIKtarget.x);
        double angle0   =  qRound(qRadiansToDegrees(rTargetX));
        //rotate base to Target angle
        emit myRobot.sigSendBoneAngle(0, angle0);

        updateBoneList();
        FABRIK_process();
        }
    QGLViewer::mouseMoveEvent(e);


}
void Viewer::mouseReleaseEvent(QMouseEvent* e)
{
    if (myMouseBehavior) {
        myMouseBehavior = false;

        vFABRIKtarget = frameEndEffector.position(); //save the EE position for FABRIK processing
        //qDebug() << endl << "Last   EE" << sV3print(myRobot.vLastPosition[E4_CLAW]);
        qDebug() << endl << "Target EE" << sV3print(vFABRIKtarget);

        //rotate to current EE angle first, this will make FABRIK process easier on 2D plane
        //double rLastEEX =  atan2(myRobot.vLastPosition[E4_CLAW].y, myRobot.vLastPosition[E4_CLAW].x);
        //double angleEEX =  qRound(qRadiansToDegrees(rLastEEX));
        double rTargetX =  atan2(vFABRIKtarget.y, vFABRIKtarget.x);
        double angle0   =  qRound(qRadiansToDegrees(rTargetX));
        qDebug() << "Pre-FABRIK: Last EE angleX, Target angleX" <<  angle0;
        emit myRobot.sigSendBoneAngle(0, angle0);

        qDebug() << "Process FABRIK:";
        updateBoneList(); //will change listBone[]
        FABRIK_process(); //will change listBone[] && listBone_[] with new target position

        double angle1=listBone_[1].angleDegree ;
        double angle2=listBone_[2].angleDegree ;
        double angle3=listBone_[3].angleDegree ;
        qDebug() << endl << "Post-FABRIK: Bone_ angles w/ Z-axis" << angle0 << angle1 << angle2 << angle3;

        //send FABRIK bone angles back to MainWindow for adjust robot angles
        //FABRIK bone angle is related to X-axis, robot angle is related to last reference angle
        double angle1Diff = angle1 - 0;
        double angle2Diff = angle2 - angle1;
        double angle3Diff = angle3 - angle2;
        bool bLimit = false;
        if (angle1Diff > degBoneAngleMax[1]) {bLimit=true; angle1Diff = degBoneAngleMax[1];}
        if (angle1Diff < degBoneAngleMin[1]) {bLimit=true; angle1Diff = degBoneAngleMin[1];}
        if (angle2Diff > degBoneAngleMax[2]) {bLimit=true; angle2Diff = degBoneAngleMax[2];}
        if (angle2Diff < degBoneAngleMin[2]) {bLimit=true; angle2Diff = degBoneAngleMin[2];}
        if (angle3Diff > degBoneAngleMax[3]) {bLimit=true; angle3Diff = degBoneAngleMax[3];}
        if (angle3Diff < degBoneAngleMin[3]) {bLimit=true; angle3Diff = degBoneAngleMin[3];}
        if (vFABRIKtarget.x <= 0.0) { //Adjust for negtive degrees @2,4 quadrants
            angle1Diff = -angle1Diff;
            angle2Diff = -angle2Diff;
            angle3Diff = -angle3Diff;
            }
        emit myRobot.sigSendBoneAngle(1, angle1Diff);
        emit myRobot.sigSendBoneAngle(2, angle2Diff);
        emit myRobot.sigSendBoneAngle(3, angle3Diff);

        Dprintf("Bone2Robot angles:%4.0f,%4.0f,%4.0f,%4.0f %s", angle0, angle1Diff, angle2Diff, angle3Diff, (bLimit?"Limited":""));
        }
    QGLViewer::mouseReleaseEvent(e);
}
/*
 * wheelEvent()
 *
 *      (0, Y) w/o ShiftModifer
 *      (X, 0) w/  ShiftModifer
 *
 */
void Viewer::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() == Qt::ShiftModifier) {
        //qDebug() << e->delta();
        Vec v  = frameEndEffector.position();   
        v.z += e->delta()/120.0; //adjust Z with mouse wheel delta
        frameEndEffector.setPosition(v);
        vFABRIKtarget = frameEndEffector.position(); //save the EE position for FABRIK processing
        updateBoneList();
        FABRIK_process();
        }
    else
        QGLViewer::wheelEvent(e);
}

//----------------------------------------------------------------------------
//////////////////////////////////  FABRIK  //////////////////////////////////

//FABRIK related
/*
 * updateBoneList()
 *
 *     update FABRIK listBone3D from robot bones 3D
 *
 */
void Viewer::updateBoneList()
{
    listBone.clear();
    for( int idx = 1; idx < ROBOT_BONES; ++idx ) {
        BONE *myBone3D = new BONE();
        myBone3D->vTail = myRobot.frame[idx-1]->position() ;
        myBone3D->vHead = myRobot.frame[idx]->position();
        listBone.append(*myBone3D);
        qDebug().noquote() << "listBone tail-head" << idx-1 << sV3print(listBone[idx-1].vTail) << sV3print(listBone[idx-1].vHead);
        }
}

void Viewer::showFABRIKvecs()
{
    //Show FABRIK listBone3D
    if (bShowFABRIKvec) {
        if (listBone.size() >0)
            for (int i=1; i<listBone.size(); i++) {
                glColor3f(0.5f, 0.3f, 1.0f);
                drawArrow(listBone[i].vTail, listBone[i].vHead, 0.04, 32);
                }
        if (listBone_.size() >0)
            for (int i=1; i<listBone_.size(); i++) {
                glColor3f(1.0f, 0.3f, 0.5f);
                drawArrow(listBone_[i].vTail, listBone_[i].vHead, 0.04, 32);
                /*
                Vec v = listBone_[i].vHead - listBone_[i].vTail;
                Vec vZ = Vec (0,0,1);
                v.normalize();
                Quaternion q = Quaternion(v, vZ); //angle with Z-axis

                glColor3f(1.0f, 1.0f, 1.0f);
                drawArrow(listBone_[i].vHead, listBone_[i].vHead+q.axis(), 0.03, 32);
                */
                }
        }
}


void Viewer::updateInfo_FABRIK_Bones_()
{
    addInfoRight(ROW_FABRIK-1, "FABRIK Bones_ Tails:");
    //T3 Base-------------------------
    Vec v         = listBone_[listBone_.size()-1].vTail; //last bone, EndEffector position
    double angle  = qAtan2(v.y, v.x);
    double angleX = qRound(qRadiansToDegrees(angle));
    QString s;
    s.sprintf(" (%+5.3f,%+5.3f,%+5.3f)  X%+4.0f°", v.x, v.y, v.z, angleX);

    listBone_[0].angleDegree = angleX;
    addInfoRight(ROW_FABRIK, s);
    qDebug().noquote() << "EE  " << s;

    //T3 Arms---------------------------
    for (int i = 1; i < listBone_.size(); ++i) {
        Vec v         = listBone_[i].vHead - listBone_[i].vTail;
        double d      = qSqrt(v.x*v.x + v.y*v.y);
        double angle  = qAtan2(d, v.z); //atan2(distance/Z)
        if (v.x <= 0.0) { angle = M_PI  - angle;} //adjust for -PI~+PI
        else            { angle = angle - M_PI;}
        double angleZ = qRound(qRadiansToDegrees(angle));

        s.sprintf("↑(%+5.3f,%+5.3f,%+5.3f)↑@Z%+4.0f°",
                  listBone_[i].vTail.x, listBone_[i].vTail.y, listBone_[i].vTail.z, angleZ);

        listBone_[i].angleDegree = angleZ;
        addInfoRight(ROW_FABRIK+i, s);

        qDebug().noquote() << "Final tail@angleZ" << s;
        }
}


/*
 * FABRIK_reach(BONE b, Vec vTo)
 *
 *  move listBone  to new goal
 *
 *      listBone.vTail~listBone.vHead => new.vTail~new.vTo
 *
 */
BONE Viewer::FABRIK_reach(BONE b, Vec vTo)
{
//qDebug().noquote() << Q_FUNC_INFO << sV3print(b.vTail) << sV3print(b.vHead) << sV3print(vTo);

    BONE boneGoal;
    // calc old bone length
    double dx = b.vTail.x - b.vHead.x;
    double dy = b.vTail.y - b.vHead.y;
    double dz = b.vTail.z - b.vHead.z;
    double lenOldBone = qSqrt(dx*dx + dy*dy + dz*dz);
    // calc vTo length && offset
    double dxTo = b.vTail.x - vTo.x;
    double dyTo = b.vTail.y - vTo.y;
    double dzTo = b.vTail.z - vTo.z;
    double lenTo= qSqrt(dxTo*dxTo + dyTo*dyTo + dzTo*dzTo);
    // cale scale
    double scale= lenOldBone/lenTo;

    //qDebug() << "lenOld lenNew scale" << lenOldBone << lenTo << scale;
    boneGoal.vHead   = vTo;
    boneGoal.vTail.x = vTo.x +dxTo*scale;
    boneGoal.vTail.y = vTo.y +dyTo*scale;
    boneGoal.vTail.z = vTo.z +dzTo*scale;
    return boneGoal;
}

/*
 *
 * https://github.com/jmysu/TuDow3-Kinematics
 *
 */
void Viewer::FABRIK_process()
{
    // Forward reaching NewBone's tail
    Vec vTgt = vFABRIKtarget;
    qDebug() << "FABRIK Target:" << sV3print(vTgt);

    for (int i=listBone.size()-1; i>0; i--) {
        BONE vNewBone = FABRIK_reach(listBone[i], vTgt); //move bone to target
        listBone[i] = vNewBone;
        vTgt = vNewBone.vTail; //looking for previous tail
        }
    // Clone listBone to listBone_ and reverse head/tail
    listBone_.clear();
    for (int i = 0; i < listBone.size(); ++i) {
        BONE b;
        b.vHead= listBone[i].vTail; //!!! IMPORTANT !!!
        b.vTail= listBone[i].vHead; //!!! IMPORTANT !!!
        listBone_.append(b);
        qDebug().noquote() << "listBone_ tail-head" << i << sV3print(listBone_[i].vTail) << sV3print(listBone_[i].vHead);
        }
    // Backward reaching NewBone's tail
    vTgt = listBone_[0].vTail; //the listBone_[0].vTail is listBone[0].vHead;
    for (int i=1; i<listBone_.size(); i++) {
        BONE vNewBone = FABRIK_reach(listBone_[i], vTgt); //move bone to target
        listBone_[i] = vNewBone;
        vTgt = vNewBone.vTail; //looking for previous tail
        }

    //End of process
    qDebug() << "FABRIK end";
    updateInfo_FABRIK_Bones_(); //update Viewer info
}

void Viewer::FABRIKtarget(double x, double y, double z)
{
    vFABRIKtarget = Vec(x,y,z); //save the EE position for FABRIK processing

    //rotate to current EE angle first, this will make FABRIK process easier on 2D plane
    double rTargetX =  atan2(vFABRIKtarget.y, vFABRIKtarget.x);
    double angle0   =  qRound(qRadiansToDegrees(rTargetX));
    //Debug() << "Pre-FABRIK: Last EE angleX, Target angleX" <<  angle0;
    emit myRobot.sigSendBoneAngle(0, angle0);

    //qDebug() << "Process FABRIK:";
    updateBoneList(); //will change listBone[]
    FABRIK_process(); //will change listBone[] && listBone_[] with new target position

    double angle1=listBone_[1].angleDegree ;
    double angle2=listBone_[2].angleDegree ;
    double angle3=listBone_[3].angleDegree ;
    //qDebug() << endl << "Post-FABRIK: Bone_ angles w/ Z-axis" << angle0 << angle1 << angle2 << angle3;

    //send FABRIK bone angles back to MainWindow for adjust robot angles
    //FABRIK bone angle is related to X-axis, robot angle is related to last reference angle
    double angle1Diff = angle1 - 0;
    double angle2Diff = angle2 - angle1;
    double angle3Diff = angle3 - angle2;
    bool bLimit = false;
    if (angle1Diff > degBoneAngleMax[1]) {bLimit=true; angle1Diff = degBoneAngleMax[1];}
    if (angle1Diff < degBoneAngleMin[1]) {bLimit=true; angle1Diff = degBoneAngleMin[1];}
    if (angle2Diff > degBoneAngleMax[2]) {bLimit=true; angle2Diff = degBoneAngleMax[2];}
    if (angle2Diff < degBoneAngleMin[2]) {bLimit=true; angle2Diff = degBoneAngleMin[2];}
    if (angle3Diff > degBoneAngleMax[3]) {bLimit=true; angle3Diff = degBoneAngleMax[3];}
    if (angle3Diff < degBoneAngleMin[3]) {bLimit=true; angle3Diff = degBoneAngleMin[3];}
    if (vFABRIKtarget.x <= 0.0) { //Adjust for negtive degrees @2,4 quadrants
        angle1Diff = -angle1Diff;
        angle2Diff = -angle2Diff;
        angle3Diff = -angle3Diff;
        }
    emit myRobot.sigSendBoneAngle(1, angle1Diff);
    emit myRobot.sigSendBoneAngle(2, angle2Diff);
    emit myRobot.sigSendBoneAngle(3, angle3Diff);

    bFABRIKok=!bLimit;
    double lenRobot = fBoneLen[E0_BASE]+fBoneLen[E1_ARM1]+fBoneLen[E2_ARM2]+fBoneLen[E3_ARM3];
    if (qSqrt(x*x+y*y+z*z) > lenRobot) bFABRIKok=false;

    Dprintf("FABRIK target(%+7.3f,%+7.3f,%+7.3f) angles:%4.0f,%4.0f,%4.0f,%4.0f %s",
            x,y,z, angle0, angle1Diff, angle2Diff, angle3Diff, (bFABRIKok ? "OK":"NG"));
}
