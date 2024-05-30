#include <stdio.h>
#include <stdint.h>
#include "STLWriter.h"


int voidCutoff = 0;
char endTag[2] = {0,0};

//Determines the normal vector of a triangle from three vertices
vect3 normalOf(vect3 p1, vect3 p2, vect3 p3){
	vect3 u = {0,0,0};
	vect3 v = {0,0,0};
	vect3 r = {0,0,0};
	u.x = p2.x-p1.x;
	u.y = p2.y-p1.y;
	u.z = p2.z-p1.z;
	v.x = p3.x-p1.x;
	v.y = p3.y-p1.y;
	v.z = p3.z-p1.z;
	r.x = u.y*v.z-u.z*v.y;
	r.y = u.z*v.x-u.x*v.z;
	r.z = u.x*v.y-u.y*v.x;
	return r;
}

//Creates a triangle and its normal vector from three vertices
triangle createTriangle(vect3 j, vect3 k, vect3 l){
	triangle t;
	t.a = j;
	t.b = k;
	t.c = l;
	t.normal = normalOf(j,k,l);
	return t;
}

//Writes a triangle into the STL file
void addTriangle(FILE * file, triangle t){
	//normal vector1
  fwrite(&t.normal.x, sizeof(float), 1, file);
  fwrite(&t.normal.y, sizeof(float), 1, file);
  fwrite(&t.normal.z, sizeof(float), 1, file);

	//vertices
  fwrite(&t.a.x, sizeof(float), 9, file);
  
  fwrite(endTag, 1, 2, file);
}

void startSTLfile(FILE * file, int numTriangles){
  rewind(file);
  //write the 80 byte STL header (can be whatever)
  for(int i = 0; i < 80; i++){
    fwrite("t",1,1,file);
  }
  //write the number of triangles (4 bytes)
  fwrite((uint32_t *)&numTriangles,4,1,file);
}

void setSTLtriangles(FILE * file, int numTriangles){
  fseek(file, 80, SEEK_SET);
  //write the number of triangles (4 bytes)
  fwrite((uint32_t *)&numTriangles,4,1,file);
}

