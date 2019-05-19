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
#ifndef GL_VIEWER_H
#define GL_VIEWER_H

#include <qglviewer.h>

#include "gl_robot.h"
#include "gl_particle.h"
#include "fabrik.h"
#include "my_utils.h"

#define ROW_FABRIK 10

using namespace qglviewer;
using namespace std;

const  int iParticleParts=125; //Particle numbers for animation

class Viewer : public QGLViewer {
public:
    GL_Robot myRobot;

    void addInfoLeft(QString s);
    void addInfoLeft(int row, QString s);
    void clearInfoLeft();
    void addInfoRight(QString s);
    void addInfoRight(int row, QString s);
    void clearInfoRight();

    bool bShowInfo=false;
    bool bShowFire=false;
    bool bShowFABRIKvec=false;
    bool myMouseBehavior=false;
    bool bShowTD3=true;
    bool bShowILoveYou=false;

    void keyPressEvent(QKeyEvent *e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent *e);

    ManipulatedFrame frameEndEffector;
    Particle *particle[iParticleParts];

    void drawFirePotBase();
    void setColorGold(bool bGold);
    void drawCone(float zMin, float zMax, float r1, float r2, int nbSub);

    void showRobotVecsArrows();
    void showProjectedXY();
    void showFireParticles();
    void showTD3Mark();
    void showILoveYou();

    //FABRIK------------------------
    QList<BONE> listBone, listBone_;
    void updateBoneList();
    void showFABRIKvecs();
    void updateInfo_FABRIK_Bones_();

    BONE FABRIK_reach(BONE b, Vec vTo);
    void FABRIK_process();

    Vec  vFABRIKtarget;
    void FABRIKtarget(double x, double y, double z);

    QPainterPath newPath;
    QList<QPolygonF> polyILoveYou;
    bool bFABRIKok = false;

protected:
  virtual void draw();
  virtual void init();
  virtual void animate();
  virtual void drawWithNames();
  virtual void postSelection(const QPoint &point);
  virtual QString helpString() const;

  void initSpotLight();

private:
  const static int iInfoLines=32;
  qglviewer::Vec orig, dir, selectedPoint; //For mouse click, selection
  void displayText();

  QString sInfoLeft[iInfoLines];
  QString sInfoRight[iInfoLines];

};

#endif
