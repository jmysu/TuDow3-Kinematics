#ifndef MY_UTILS_H
#define MY_UTILS_H

#include <qglviewer.h>
using namespace qglviewer;
using namespace std;

extern QString sV3print(const Vec v);
extern QString sM4x4Print(const GLdouble *m);
extern double VertexAngle(Vec v1, Vec v2);

#define Dprintf(fmt, ...) fprintf(stderr, "%s() @%i: " fmt "\n", __FUNCTION__, __LINE__, __VA_ARGS__)

#endif
