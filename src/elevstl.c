#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "STLWriter.h"
#include "elevation.h"


// gcc elevstl.c STLWriter.c elevation.c -o celevstl -lm

float globalLat = 0;
const float  PI=3.14159265358979f;

int writeLineWall(FILE * file, float * heights, int width, float xScale, float yval, int flipNormal){
	int numtris = 0;

	for(int i = 1; i <width; i++){
		if(heights[i]>voidCutoff && heights[i-1]>voidCutoff){
			struct _vect3 a = {i*xScale,     yval,heights[i]};
			struct _vect3 b = {(i-1)*xScale, yval,heights[i-1]};

			struct _vect3 w = {i*xScale,     yval, 0};
			struct _vect3 z = {(i-1)*xScale, yval, 0};

			if(flipNormal){
				addTriangle(file, createTriangle(z,b,a));			//model walls
				addTriangle(file, createTriangle(a,w,z));
			}else{
				addTriangle(file, createTriangle(b,z,a));			//model walls
				addTriangle(file, createTriangle(w,a,z));
			}

			numtris += 2;
		}
	}

	return numtris;
}

//lh is lower heights, ie those along a strip with a lower y component
//uh is upper height, those along a strip with a higher y component
int writeXStrip(FILE * file, float * lh, float * uh, int width, float xScale, float lyval, float hyval){
	int numtris = 0;

	//build left wall
	struct _vect3 u =  {0,hyval, uh[0]};
	struct _vect3 l =  {0,lyval, lh[0]};
	struct _vect3 ub = {0,hyval,  0};
	struct _vect3 lb = {0,lyval,  0};
	addTriangle(file, createTriangle(u,ub,lb));
	addTriangle(file, createTriangle(u,lb,l));
	numtris += 2;

	//build right wall
	u =  (struct _vect3){(width-1)*xScale,hyval, uh[width-1]};
	l =  (struct _vect3){(width-1)*xScale,lyval, lh[width-1]};
	ub = (struct _vect3){(width-1)*xScale,hyval,  0};
	lb = (struct _vect3){(width-1)*xScale,lyval,  0};
	addTriangle(file, createTriangle(u,lb,ub));
	addTriangle(file, createTriangle(u,l,lb));
	numtris += 2;

	for(int x = 1; x < width; x++){
		float ha = uh[x];
		float hb = lh[x];             // d---a
		float hc = lh[x-1];           // |   |
		float hd = uh[x-1];           // c---b

		if(ha>voidCutoff && hb>voidCutoff && hc>voidCutoff && hd>voidCutoff){
			struct _vect3 a = {x*xScale,    hyval,ha};
			struct _vect3 b = {x*xScale,    lyval,hb};
			struct _vect3 c = {(x-1)*xScale,lyval,hc};
			struct _vect3 d = {(x-1)*xScale,hyval,hd};

			//choose where to split the square based on local curvature
			if( fabs(hd-hb) < fabs(ha-hc) ){
				addTriangle(file, createTriangle(a,d,b));
				addTriangle(file, createTriangle(c,b,d));
			}else{
				addTriangle(file, createTriangle(a,d,c));
				addTriangle(file, createTriangle(a,c,b));
			}
			numtris += 2;
		}
	}

	return numtris;
}


int main(int argc, char **argv)			//lat, long, width, height, verticalscale, rot, waterDrop, baseHeight, stepSize, outputname
//width and height are in units of steps or maybe degrees??
//rot is in degrees
{
	if(argc!=11){
		printf("Got %d arguments, expected 10:\n", argc-1);
		printf("%s lat long width height verticalscale rot waterDrop baseHeight stepSize outputname\n", argv[1]);
		return -1;
	}
	float lat;
	float lng;
	int width;
	int height;
	float userscale;
	float rot;
	int stepSize = 1;
	int waterDrop = -2;			//millimeters
	int baseHeight = 2;			//millimeters

	//float true_verticalscale = 92.7;	//meters/arcsecond at equator
	//old vertical scale was 23.2
	float verticalscale = 92.7;			//true_verticalscale gives models that are too flat to be interesting

	lat = atof(argv[1]);					//Latitude of NW corner
	globalLat = 3.1415926*lat/180;
	lng = atof(argv[2]);					//Longitude of NW corner
	printf("'Northwest' coordinate: (%.6f N, %.6f E)\n", lat, lng);
	width = atoi(argv[3]);
	height = atoi(argv[4]);
	userscale = atof(argv[5]);
	rot = atof(argv[6]);
	rot = rot*PI/180;

	waterDrop = atoi(argv[7]);
	baseHeight = atoi(argv[8]);

	stepSize = atoi(argv[9]);

	char * outputName = argv[10];

	printf("Step size: %d units\n", stepSize);

	float scaleFactor = (userscale/verticalscale) / ((float) stepSize);

	////////////// Done with Argument Parsing //////////////////

	//allocate memory for two lines of heights
	float * nextline = (float *) calloc(width, sizeof(float));
	float * prevline = (float *) calloc(width, sizeof(float));

	if(nextline==NULL || prevline==NULL){
		printf("Allocation error!");
		return -1;
	}

	int tris = 0;
	FILE* stl = fopen(outputName, "w");
	startSTLfile(stl, 4);

	//get zeroth line
	getElevationLine(nextline, width, -height, lat, lng, scaleFactor, rot, waterDrop,baseHeight, stepSize);
	tris += writeLineWall(stl, nextline, width, cos(globalLat)/3, -((float)height)/3, 0);

	for(int y = -height+1; y<=0; y++){
		for(int x = 0; x<width; x++){
			prevline[x] = nextline[x];
		}
		getElevationLine(nextline, width, y, lat, lng, scaleFactor, rot, waterDrop,baseHeight, stepSize);
		tris += writeXStrip(stl, prevline, nextline, width, cos(globalLat)/3, ((float)(y-1))/3, ((float)y)/3);
		fflush(stl);
	}

	//write other x wall
	tris += writeLineWall(stl, nextline, width, cos(globalLat)/3, 0, 1.0/3.0);


	// add in the bottom of the model
	float xScale = cos(globalLat);
	struct _vect3 o =  {10,-10, 0};
	for(int y = -height+1; y<=0; y++){
		struct _vect3 lowerleft = {0,y-1, 0};
		struct _vect3 upperleft = {0,y,   0};
		struct _vect3 lowerright = {(width-1)*cos(globalLat),y-1, 0};
		struct _vect3 upperright = {(width-1)*cos(globalLat),y,   0};
		addTriangle(stl, createTriangle(lowerleft,upperleft,o));
		addTriangle(stl, createTriangle(upperright,lowerright,o));
		tris += 2;
	}
	for(int x = 0; x<width-1; x++){
		struct _vect3 upperleft =  {x*xScale,0, 0};
		struct _vect3 upperright = {(x+1)*xScale,0,   0};
		struct _vect3 lowerleft = {x*xScale,-height, 0};
		struct _vect3 lowerright =  {(x+1)*xScale,-height,   0};
		addTriangle(stl, createTriangle(upperleft,upperright,o));
		addTriangle(stl, createTriangle(lowerright,lowerleft,o));
		tris += 2;
	}

	//set the number of triangles in the header to tris
	setSTLtriangles(stl, tris);
	fclose(stl);

	//passing global lat as an xscale - only needed for
	//writeSTLfromArray(hList,width,height,globalLat);
	//passing zero as latittude to bypass lat compensation
	//writeSTLfromArray(hList,width,height,globalLat);
	return 0;
}
