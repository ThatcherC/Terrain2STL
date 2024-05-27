#ifndef STLWRITER_H
#define STLWRITER_H

#include <stdio.h>

extern int voidCutoff;

typedef struct _vect3 {
  float x;
  float y;
  float z;
} vect3;

typedef struct _triangle {
  vect3 a;
  vect3 b;
  vect3 c;
  vect3 normal;
} triangle;

//typedef struct _vect3 vect3;
//typedef struct _triangle triangle;

vect3 normalOf(vect3, vect3, vect3);
triangle createTriangle(vect3, vect3, vect3);
void addTriangle(FILE *, triangle t);
void startSTLfile(FILE *, int);
void setSTLtriangles(FILE *, int);

//void writeSTLfromArray(const std::vector<float>&, int, int, float);

#endif // #STLWRITER_H
