#ifndef DH_H
#define DH_H

/*
 *
 *  https://en.wikipedia.org/wiki/Denavit%E2%80%93Hartenberg_parameters
 *
 *  Side view------------------------
 *  dHeightZ
 *      d - the distance between the previous x-axis and the current x-axis, along the previous z-axis.
 *
 *  alphaZ
 *      α - the angle between the previous z-axis and current z-axis.

 *  Top view-----------------------
 *  rOffsetX
 *      a (or r) - the distance between the previous z-axis and the current z-axis
 *
 *  thetaX
 *      θ - the angle around the z-axis between the previous x-axis and current x-axis.
 *

 T3 Robot
        +---------+--------+----------+--------+
        | dHeightZ| alphaZ |  rOffsetX| thetaX |
        +---------+--------+----------+--------+
 Base       0.1       0          0       -180
 Arm1       0.3     -60          0          0
 Arm2       0.2      30          0          0
 Arm3       0.2      30          0          0
 Claw       0.2       0          0          0


 T(i-1,i) = | Cos(theta) -Sin(theta)Cos(alpha) Sin(theta)Sin(alpha)  rCos(theta) |
            | Sin(theta) Cos(theta)Cos(alpha)  -Cos(theta)Sin(alpha) rSin(theta) |
            |     0            Sin(alpha)           Cos(alpha)           d       |
            |     0                0                     0               1       |


 *
 */
#include <QtCore>

#define DH_LINKS 5
const QStringList _lLinkname = {"Base","Arm1","Arm2","Arm3","Claw"};
const double _d[DH_LINKS]    = {   0.1,   0.3,   0.2,   0.2,   0.2}; //dHeight
const double _a[DH_LINKS]    = {     0,   -60,    30,    30,     0}; //alpha
const double _r[DH_LINKS]    = {     0,     0,     0,     0,     0}; //rOffset
const double _t[DH_LINKS]    = {    90,     0,     0,     0,     0}; //theta

class DH
{
public:
    DH();    
    //--------------------------------------------------------
    double alphaZ;   //rotation,     side view
    double dHeightZ; //translation,  side view
    double rOffsetX; //translation,  top view
    double thetaX;   //rotation,     top view
};

#endif // DH_H
