#include <iostream>
#include <stdint.h>
#include <fstream>
#include <cmath>
#include "stdio.h"
#include "stdlib.h"
#include <vector>
#include "Vector.h"
#include "LatLng.h"
#include "STLWriter.h"
#include "Elevation.h"


// g++ elevstl.cpp Vector.cpp LatLng.cpp STLWriter.cpp Elevation.cpp -o elevstl -std=c++0x

using namespace std;

float globalLat = 0;
vector<float> hList;
const float  PI=3.14159265358979f;

int main(int argc, char **argv)			//lat, long, width, height, verticalscale, rot, waterDrop, baseHeight, stepSize
//width and height are in units of steps or maybe degrees??
//rot is in degrees
{
	float lat;
	float lng;
	int width;
	int height;
	float userscale;
	float rot;
	int stepSize = 1;
	int waterDrop = -2;			//millimeters
	int baseHeight = 2;			//millimeters
	int stepSize = 1;

	//float true_verticalscale = 92.7;	//meters/arcsecond at equator
	//old vertical scale was 23.2
	float verticalscale = 92.7;			//true_verticalscale gives models that are too flat to be interesting

	lat = atof(argv[1]);					//Latitude of NW corner
	globalLat = 3.1415926*lat/180;
	lng = atof(argv[2]);					//Longitude of NW corner
	clog << "'Northwest' coordinate: (" << lat << "N, "<< lng << "E)\n";
	width = atoi(argv[3]);
	height = atoi(argv[4]);
	userscale = atof(argv[5]);
	rot = atof(argv[6]);
	rot = rot*PI/180;

	hList.resize(width*height,0);

	waterDrop = atoi(argv[7]);
	baseHeight = atoi(argv[8]);

	stepSize = atoi(argv[9]);
	clog << "Step size: " << stepSize << " units\n";

	float scaleFactor = (userscale/verticalscale) / ((float) stepSize);

	hList = getElevations(lat,lng,width,height,scaleFactor,rot,waterDrop,baseHeight,stepSize);

	//passing global lat as an xscale - only needed for
	//writeSTLfromArray(hList,width,height,globalLat);
	//passing zero as latittude to bypass lat compensation
	writeSTLfromArray(hList,width,height,globalLat);
	return 0;
}
