#ifndef COMPLEX_H
#define COMPLEX_H

#include <math.h>

typedef struct _comp
{
    double x;
    double y;
} Comp;

#define initComplex(vx,vy,z) z.x=vx;z.y=vy
#define assign(a,b) a.x=b.x;a.y=b.y
#define squareAdd(a,c,aux) aux.x=(a.x*a.x-a.y*a.y);aux.y=2*a.x*a.y;aux.x=aux.x+c.x;aux.y=aux.y+c.y
#define squareAddAssign(a,c) {double temp_x=a.x*a.x-a.y*a.y; double temp_y=2*a.x*a.y; a.x=temp_x+c.x; a.y=temp_y+c.y;}
#define md(a) sqrt(a.x*a.x+a.y*a.y)
#define mdSquared(a) (a.x*a.x+a.y*a.y)
#define mdr(a) abs(a.x+a.y)
#define diff(a,b,aux) aux.x=a.x-b.x;aux.y=a.y-b.y;

#endif // COMPLEX_H
