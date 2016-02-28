//Vector.cpp:
//Defines a 3D vector class
#include "Vector.h"
#include "LatLng.h"
#include <iostream>
#include <cmath>

const float  PI=3.14159265358979f;
const float r2d = 180/PI;

Vector::Vector(float _x, float _y, float _z){
  x = _x;
  y = _y;
  z = _z;
}

Vector::Vector(void){
  x = 0;
  y = 0;
  z = 0;
}

Vector Vector::add(const Vector &v){
  return Vector(x+v.x,y+v.y,z+v.z);
}

Vector Vector::subtract(const Vector &v){
  return Vector(x-v.x,y-v.y,z-v.z);
}

Vector Vector::multiply(float s){
  return Vector(x*s,y*s,z*s);
}

LatLng Vector::toSpherical(void){
  float lat = atan2(z,sqrt(x*x+y*y));
  float lng = atan2(y,x);
  return LatLng(lat*r2d, lng*r2d);
}

void Vector::print(){
  std::clog << "<" << x << ", " << y << ", " << z << ">\n";
}
