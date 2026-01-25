#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "STLWriter.h"


int voidCutoff = 0;
char endTag[2] = {0,0};

// ============== Buffered Writer Implementation ==============

// fwrite-like interface: write to buffer instead of file
size_t bufwrite(const void *ptr, size_t size, size_t count, STLWriter *w) {
    size_t bytes = size * count;

    // Flush if this write would overflow
    if (w->pos + bytes > w->capacity) {
        stlwriter_flush(w);
    }

    memcpy(w->buffer + w->pos, ptr, bytes);
    w->pos += bytes;
    return count;
}

STLWriter *stlwriter_create(FILE *file) {
    STLWriter *w = (STLWriter *)malloc(sizeof(STLWriter));
    w->file = file;
    w->buffer = (char *)malloc(STL_BUFFER_SIZE);
    w->pos = 0;
    w->capacity = STL_BUFFER_SIZE;
    w->triCount = 0;
    return w;
}

void stlwriter_flush(STLWriter *w) {
    if (w->pos > 0) {
        fwrite(w->buffer, 1, w->pos, w->file);
        w->pos = 0;
    }
}

void stlwriter_free(STLWriter *w) {
    if (w) {
        free(w->buffer);
        free(w);
    }
}

void addTriangleBuffered(STLWriter *w, triangle t) {
    // normal vector
    bufwrite(&t.normal.x, sizeof(float), 1, w);
    bufwrite(&t.normal.y, sizeof(float), 1, w);
    bufwrite(&t.normal.z, sizeof(float), 1, w);

    // vertices (9 floats: a.x,a.y,a.z, b.x,b.y,b.z, c.x,c.y,c.z)
    bufwrite(&t.a.x, sizeof(float), 9, w);

    // attribute byte count (2 bytes, unused)
    bufwrite(endTag, 1, 2, w);

    w->triCount++;
}

void startSTLfileBuffered(STLWriter *w) {
    // Write 80-byte header
    char header[80];
    memset(header, 't', 80);
    bufwrite(header, 1, 80, w);

    // Write placeholder for triangle count (will update at end)
    uint32_t placeholder = 0;
    bufwrite(&placeholder, 4, 1, w);
}

void finalizeSTLfileBuffered(STLWriter *w) {
    // Flush any remaining data
    stlwriter_flush(w);

    // Go back and write the actual triangle count
    fseek(w->file, 80, SEEK_SET);
    uint32_t count = (uint32_t)w->triCount;
    fwrite(&count, 4, 1, w->file);
}

// ============== Original (unbuffered) Implementation ==============

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

