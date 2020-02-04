#include <stdio.h>
#include "STLWriter.h"

// gcc stltest.c STLWriter.c -o stltest -lm

int main(){
  struct _vect3 o = {0,0,0};
  struct _vect3 x = {1,0,0};
  struct _vect3 y = {0,1,0};
  struct _vect3 z = {0,0,1};

  triangle xy = createTriangle(o,y,x);
  triangle yz = createTriangle(o,z,y);
  triangle zx = createTriangle(o,x,z);
  triangle xyz = createTriangle(x,y,z);

  FILE *stl = fopen("corner.stl", "w");

  startSTLfile(stl, 1);

  addTriangle(stl, xy);
  addTriangle(stl, yz);
  addTriangle(stl, zx);
  addTriangle(stl, xyz);

  setSTLtriangles(stl, 4);

  fclose(stl);
}
