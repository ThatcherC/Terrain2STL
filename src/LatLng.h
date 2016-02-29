#ifndef LATLNG_H
#define LATLNG_H

class Vector;

class LatLng {
  public:
    float lat,lng;
    LatLng (float,float);
    void print();
    Vector toCartesian(void);
};

#endif
