#ifndef ELEVATION_H
#define ELEVATION_H

#include <stdio.h>
#include <math.h>

int getTile(char * filename, size_t len, float lat, float lng);
int getTileIndex(float lat, float lng);
int getElevationLine(float* heights, int width, int nthLine, float startLat,float startLng, float vscale,float rot, int waterDrop, int baseHeight, int stepSize);
//vector<float> getElevations(float _lat,float _lng,int width,int height,float vscale,float rot,int,int,int);

#endif // #ELEVATION_H
