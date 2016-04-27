//Elevation.cpp
//Gets elevations at points and in regions

#include <string>
#include <math.h>
#include <fstream>
#include <vector>
#include <iostream>
#include "Elevation.h"

string getTile(float lat, float lng){
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
	file.insert(0,"./hgt_files/");

	return file;
}

//returns the 'index' of the tile that contains a certain coordinate
//quicker than getTile()
int getTileIndex(float lat, float lng){
	return (int)floor(lat)*360 + (int)floor(lng);
}


//width and heigth are in 'pixels'
vector<float> getElevations(float _lat,float _lng,int width,int height,float vscale,float rot, int waterDrop, int baseHeight){
	int tileNumber = 0;
	string tileName = "";
	ifstream file;
	vector<float> outList(width*height);

	file.open("../Terrain2STL-master/hgt_files/N44W070.hgt");

	int h;
	char number [2];

	int t = getTileIndex(_lat,_lng);

	//let's go with mercator space for now (Terrain2STL style)
	for(int x = 0; x<width; x++){
		for(int y = 0; y<height; y++){
			float u = (float)y/1200 - (float)height/1200;
			float v = (float)x/1200;

			//get the lat and lng for each point
			float lat = _lat + u*cos(rot) + v*sin(rot);
			float lng = _lng + v*cos(rot) - u*sin(rot);

			//interpolate - maybe skip for rot0=0?

			float elevations[2][2];
			for(int a = 0; a < 2; a++){		//x coord for Interpolation
				for(int b = 0; b < 2; b++){ //

					//interesting bit of code here.
					//floor(...)/1200 as no distortion but cannot handle tile edges
					//x+float() handles edges well but with distortion
					//lat+(float)b/1200
					float intlat = floor(lat*1200+b)/1200.0+0.000598907;	//magic number??
					float intlng = floor(lng*1200+a)/1200.0;
					//floor(lng*1200+a)/1200;

					if(getTileIndex(intlat, intlng)!=tileNumber){
						tileNumber = getTileIndex(intlat, intlng);
						tileName = getTile(intlat, intlng);

						file.close();
			      file.open(tileName.c_str(),ios::in|ios::binary);
					}

					int p = (int) (1201*(intlng-floor(intlng)));		//x or lng component
					p += (int)(1201*(ceil(intlat)-intlat))* 1201;	//y or lat component

					file.seekg(p*2,ios::beg);
			    file.read(number,2);
			    h = number[1];
			    if(h<0){
			      h = h+255;
			    }
			    h+= number[0]*256;
					if(h==0){
						h-= waterDrop/vscale;
					}
					elevations[a][b]= (float)h;
				}
			}
			float fracLat = lat-floor(lat*1200)/1200;
			float fracLng = lng-floor(lng*1200)/1200;
			fracLat *= 1200;
			fracLng *= 1200;
			float westLng = elevations[0][0]*(1-fracLat)  +  elevations[0][1]*fracLat;
			float eastLng = elevations[1][0]*(1-fracLat)  +  elevations[1][1]*fracLat;

			//not sure about this....
			float intHeight = westLng*(1-fracLng) + eastLng*fracLng;

			//if(t!=getTileIndex(lat,lng)){
			//	intHeight=0;
			//}
			//float intHeight = (elevations[0][0]+elevations[0][1]+elevations[1][0]+elevations[1][1])/4;
			outList.at(y*width+x) = intHeight*vscale+baseHeight;
		}
	}
	file.close();
	clog << "Done getting elevations\n";
	return outList;
}







/*
//-------Find starting file index---------------
float n = (lat-floor(lat))*3600;
float e = (lng-floor(lng))*3600;

int i = 1201-(int)(n/(3));
int j = (int)(e/(3));

int tilesOffsetX = 10000;	//how much the secondary x tiles should be offset in x, 10000 means only one tile is used
int tilesOffsetY = 10000;	//how much the secondary x tiles should be offset in x, 10000 means only one tile is used
if(i+height*stepSize>1200){
  clog << "Extra tile in y\n";
  tilesOffsetY = (1200-i);
  getTile(lat-1.0,lng,2);
}
if(j+width*stepSize>1200){
  clog << "Extra tile in x\n";
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
*/
