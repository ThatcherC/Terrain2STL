#ifndef VECTOR_H
#define VECTOR_H

class LatLng;

class Vector {
  public:
    float x,y,z;
    Vector (float,float,float);
    Vector (void);
    Vector add(const Vector&);
    Vector subtract(const Vector&);
    Vector multiply(float);
    void print();
    LatLng toSpherical(void);
};

#endif
