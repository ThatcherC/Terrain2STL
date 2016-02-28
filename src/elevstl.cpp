#include <iostream>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include <string>
#include <vector>
#include "Vector.h"
#include "LatLng.h"
#include "STLWriter.h"


// g++ elevstl.cpp Vector.cpp LatLng.cpp STLWriter.cpp -o elevstl -std=c++0x

using namespace std;

string tiles[4];

void getTile(float lat, float lng, int index){
	string file;
	//-------get correct tile-------------------
	if(lat>=0){								//Positive is north
		file = "N";
	}else{
		file = "S";
	}
	file.append( to_string( abs( (int)floor(lat) ) ) );
	if(file.length()==2) file.insert(1,"0");
	if(lng>=0){								//Positive is east
		file.append("E");
	}else{
		file.append("W");
	}
	file.append( to_string( abs( (int)floor(lng) ) ) );
	if(file.length()==5)file.insert(4,"00");
	if(file.length()==6)file.insert(4,"0");

	file.append(".hgt");
	file.insert(0,"../Terrain2STL/hgt_files/");
	tiles[index] = file;
}

int width = 40;
int height = 40;
float globalLat = 0;
vector<float> hList;

int main(int argc, char **argv)			//lat, long, res, filename, waterDrop, baseHeight
{
	int point;
	float lat;
	float lng;
	int res;
	int stepSize = 1;
	int tile_n;
	int tile_w;
	int waterDrop = -2;			//millimeters
	int baseHeight = 2;			//millimeters
	//float true_verticalscale = 92.7;	//meters/arcsecond at equator
	//old vertical scale was 23.2
	float verticalscale = 92.7;			//true_verticalscale gives models that are too flat to be interesting

	lat = atof(argv[1]);					//Latitude of NW corner
	clog << "Using latitude: " <<lat;
	globalLat = 3.1415926*lat/180;
	lng = atof(argv[2]);					//Longitude of NW corner
	clog << "Using longitude: " << lng;
	res = atoi(argv[3]);					//arcseconds/tick in model
	if(res%3!=0){							//must be a multiple of 3
		clog << "Bad resolution\n";
		return 0;
	}
	res = res/3;							//SRTM data already has a resolution of 3 arcseconds
	if(res>15){
		stepSize = 8;
	}else if(res>12){
		stepSize = 4;
	}else if(res>9){
		stepSize = 2;
	}
	clog << "Stepsize: " << stepSize;
	clog << "Using resolution " << res;	//Arc
	height = 40*res/stepSize;
	width = 40*res/stepSize;

	hList.resize(width*height,0);

	//savefile = std::string(argv[4]))

	waterDrop = atoi(argv[5]);
	baseHeight = atoi(argv[6]);
	getTile(lat,lng,0);

	//-------Find starting file index---------------
	float n = (lat-floor(lat))*3600;
	float e = (lng-floor(lng))*3600;

	int i = 1201-(int)(n/(3));
	int j = (int)(e/(3));

	int tilesOffsetX = 10000;	//how much the secondary x tiles should be offset in x, 10000 means only one tile is used
	int tilesOffsetY = 10000;	//how much the secondary x tiles should be offset in x, 10000 means only one tile is used
	if(i+height*stepSize>1200){
		clog << "Extra tile in y";
		tilesOffsetY = (1200-i);
		getTile(lat-1.0,lng,2);
	}
	if(j+width*stepSize>1200){
		clog << "Extra tile in x";
		tilesOffsetX = (1200-j);
		getTile(lat,lng+1.0,1);
		if(tilesOffsetY!=10000){
			getTile(lat-1.0,lng+1.0,3);
		}
	}

	point = j+i*1201;						//the file index of the NW corner

	//------------Open file and read data into array----------------------------


	int h;
	char number [2];

	int whichTile;
	int tileX;
	int tileY;
	ifstream file;
	int openTile = -1;;

	for(int y = 0; y < height*stepSize; y+=stepSize){
		for(int x = 0; x < width*stepSize; x+=stepSize){
			tileX = x;
			tileY = y;
			if(x<=tilesOffsetX && y<=tilesOffsetY){			//simplify this with bitwise logic?
				whichTile = 0;
				point = j+i*1201;
			}
			if(x>tilesOffsetX && y<=tilesOffsetY){
				whichTile = 1;
				tileX = x-tilesOffsetX;
				point = i*1201;
			}
			if(x<=tilesOffsetX && y>tilesOffsetY){
				whichTile = 2;
				tileY = y-tilesOffsetY;
				point = j;
			}
			if(x>tilesOffsetX && y>tilesOffsetY){
				whichTile = 3;
				tileX = x-tilesOffsetX;
				tileY = y-tilesOffsetY;
				point = 0;
			}

			if(openTile!=whichTile){
				openTile = whichTile;
				file.close();
				file.open(tiles[whichTile].c_str(),ios::in|ios::binary);
			}


			file.seekg((point+tileX+tileY*1201)*2,ios::beg);
			file.read(number,2);
			h = number[1];
			if(h<0){
				h = h+255;
			}
			h+= number[0]*256;

			if(h==0){
				h=-waterDrop*verticalscale;
			}

			if(h<-1000){
				h=-verticalscale*2000*stepSize;
			}
			//rotate model to correct orientation
			//hList.at((height-1-y)*width+x) = h/(verticalscale*res); //cast verticalscale to int for COOl effect!
			hList.at((height-1-y/stepSize)*width+x/stepSize) = h/(verticalscale*stepSize)+baseHeight; 	//+baseHeight so that the bottom of the model does not bleed through to the top
		}
	}

	//passing global lat as an xscale - only needed for
	writeSTLfromArray(hList,width,height,globalLat);
	return 0;
}
