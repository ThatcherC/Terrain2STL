//LatLng.cpp:
//Defines a latitude-longitude coordinates point
#include "LatLng.h"
#include "Vector.h"
#include <iostream>
#include <cmath>

const float  PI=3.14159265358979f;
const float d2r = PI/180;

LatLng::LatLng(float _lat, float _lng){
  lat = _lat;
  lng = _lng;
}

Vector LatLng::toCartesian(void){
  float x = cos(lat*d2r)*cos(lng*d2r);
  float y = cos(lat*d2r)*sin(lng*d2r);
  float z = sin(lat*d2r);
  return Vector(x,y,z);
}

void LatLng::print(void){
  std::clog << "[" << lat << ", " << lng << "]\n";
}
