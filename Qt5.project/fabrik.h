#ifndef FABRIK_H
#define FABRIK_H

#include "gl_robot.h"

/*
 * FABRIK Bone
 *
 *  the vector with start and stop point on plane XZ (Y=0)
 *  and with angles in degree
 */
class BONE
{
public:
    Vec vTail;         // start point
    Vec vHead;         // stop point
    double angleDegree;//
};
#endif // FABRIK_H
