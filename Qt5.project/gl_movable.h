

#include "gl_robot.h"
#include <math.h>
#include <QDebug>

using namespace qglviewer;
using namespace std;

class MovableObject : public MouseGrabber
{
public:
    qglviewer::Frame frameMovable;

    MovableObject() : pos(0,0), moved(false) { }

    void checkIfGrabsMouse(int x, int y, const qglviewer::Camera* const camera) {
        // MovableObject is active in a region of 5 pixels around its pos.
        // May depend on the actual shape of the object. Customize as desired.
        // Once clicked (moved = true), it keeps grabbing mouse until button is released.
        //setGrabsMouse( moved || ((pos-QPoint(x,y)).manhattanLength() < 15) );
        Vec proj = camera->projectedCoordinatesOf(mf_.position());

        bool bInRange = (fabs(x-proj.x) < 10) && (fabs(y-proj.y) < 8);
        setGrabsMouse(moved || bInRange); // Rectangular
        qDebug() <<proj.x << proj.y << QPoint(x,y) << bInRange;
        }

     void mousePressEvent( QMouseEvent* const e, Camera* const) { prevPos = e->pos(); moved = true; }

    void mouseMoveEvent(QMouseEvent* const e, const Camera* const) {
        if (moved) {
            // Add position delta to current pos
            pos += e->pos() - prevPos;
            prevPos = e->pos();
            qDebug() << pos;
            }

        }

    void mouseReleaseEvent(QMouseEvent* const, Camera* const) { moved = false; }


    void draw() {
        // The object is drawn centered on its pos, with different possible aspects:
        if (grabsMouse())
        if (moved) {
            // Object being moved, maybe a transparent display
            //else
            // Object ready to be moved, maybe a highlighted visual feedback
            //mf_.setPosition(Vec(pos.x(),pos.y(),0)); //init teapot position
            }
        else {
            // Normal display
            glPushMatrix();
            glMultMatrixd(mf_.matrix());
            glColor3f(212.0f/255.0f, 175.0f/255.0f, 55.0f/255.0f); //Gold

            //float fLen = 0.25f;
            //drawCone(      0.0, fLen* 6/10, 0.1f,  0.1f,  32); //base cylinder
            //drawCone(fLen*6/10, fLen* 8/10, 0.1f,  0.05f, 32);
            //drawCone(fLen*8/10, fLen*10/10, 0.05f, 0.01f, 32);
            //Draw Crosshair
            //glColor3f(0.35f, 0.35f, 0.35f);
            glLineWidth(6);
            glBegin(GL_LINES);
            Vec v;
            v.x = pos.x(); v.y= pos.y(); v.z=0;
            glVertex2d(v.x-0.25, v.y-0.25);
            glVertex2d(v.x+0.25, v.y+0.25);
            glVertex2d(v.x-0.25, v.y+0.25);
            glVertex2d(v.x+0.25, v.y-0.25);
            glEnd();

            glPopMatrix();
            }

        //Draw Crosshair
        glColor3f(0.35f, 0.35f, 0.35f);
        glLineWidth(6);
        glBegin(GL_LINES);
        Vec v;
        v.x = pos.x(); v.y= pos.y(); v.z=0;
        glVertex2d(v.x-0.25, v.y-0.25);
        glVertex2d(v.x+0.25, v.y+0.25);
        glVertex2d(v.x-0.25, v.y+0.25);
        glVertex2d(v.x+0.25, v.y-0.25);
        glEnd();
        }

    void setPosition(const qglviewer::Vec &pos) { mf_.setPosition(pos); }

  private:
        qglviewer::ManipulatedFrame mf_;

        QPoint pos, prevPos;
        bool moved;
        // Draws a truncated cone aligned with the Z axis.
        void drawCone(float zMin, float zMax, float r1, float r2, int nbSub) {
            static GLUquadric *quadric = gluNewQuadric();

            glTranslatef(0.0, 0.0, zMin);
            gluCylinder(quadric, r1, r2, zMax - zMin, nbSub, 1);
             glTranslatef(0.0, 0.0, -zMin);
            }
};
