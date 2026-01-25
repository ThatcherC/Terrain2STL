#include <stdio.h>
#include <stddef.h>

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

// Buffered STL writer
#define STL_BUFFER_SIZE (10000 * 50)  // 10k triangles * 50 bytes each

typedef struct {
    FILE *file;
    char *buffer;
    size_t pos;       // current position in buffer
    size_t capacity;  // buffer capacity
    int triCount;     // total triangles written
} STLWriter;

// fwrite-like interface for writing to buffer
size_t bufwrite(const void *ptr, size_t size, size_t count, STLWriter *w);

// Buffered writer functions
STLWriter *stlwriter_create(FILE *file);
void stlwriter_flush(STLWriter *w);
void stlwriter_free(STLWriter *w);
void addTriangleBuffered(STLWriter *w, triangle t);
void startSTLfileBuffered(STLWriter *w);
void finalizeSTLfileBuffered(STLWriter *w);

//void writeSTLfromArray(const std::vector<float>&, int, int, float);
