#ifndef STLWRITER1A_H
#define STLWRITER1A_H

#include <vector>

struct triangle;

Vector normalOf(const Vector&, const Vector&, const Vector&);
triangle createTriangle(const Vector&, const Vector&, const Vector&);
void addTriangle(triangle t);

void writeSTLfromArray(const std::vector<float>&, int, int, float);

#endif
