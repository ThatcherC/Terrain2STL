#ifndef ELEVATION_H
#define ELEVATION_H

#include <vector>
#include <string>

using namespace std;

string getTile(float, float);
int getTileIndex(float lat, float lng);
vector<float> getElevations(float _lat,float _lng,int width,int height,float vscale,float rot,int,int);

#endif
