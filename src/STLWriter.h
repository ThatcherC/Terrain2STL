#ifndef STLWRITER_H
#define STLWRITER_H

#include <vector>

struct triangle;

Vector normalOf(const Vector&, const Vector&, const Vector&);
triangle createTriangle(const Vector&, const Vector&, const Vector&);
void addTriangle(triangle t);

void writeSTLfromArray(const std::vector<float>&, int, int);

#endif
