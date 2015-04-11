#include <iostream>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include <string>
#include <vector>


// g++ elevstl.cpp -o elevstl -std=c++0x

using namespace std;

struct vertex{		//this really ends up being used as a vector--
	float x;		//could probably just use a vector instead
	float y;
	float z;
};

struct triangle{
	vertex a;
	vertex b;
	vertex c;
	vertex normal;
};

char endTag[2] = {0,0};
ofstream out;

//Determines the normal vector of a triangle from three vertices
vertex normalOf(vertex p1, vertex p2, vertex p3){
	vertex u,v,r;
	u.x = p2.x-p1.x;
	u.y = p2.y-p1.y;
	u.z = p2.z-p1.z;
	v.x = p3.x-p1.x;
	v.y = p3.y-p1.y;
	v.z = p3.z-p1.z;
	r.x = u.y*v.z-u.z*v.y;
	r.y = u.z*v.x-u.x*v.z;
	r.z = u.x*v.y-u.y*v.x;
	return r;
}

//Creates a vertex from x, y, and z coordinates
vertex createVertex(float x, float y, float z){
	vertex v;
	v.x =x;
	v.y = y;
	v.z = z;
	return v;
}

//Creates a triangle and its normal vector from three vertices
triangle createTriangle(vertex j, vertex k, vertex l){
	triangle t;
	t.a = j;
	t.b = k;
	t.c = l;
	t.normal = normalOf(j,k,l);
	return t;
}
	
//Writes a triangle into the STL file
void addTriangle(triangle t){
	//normal vector1
	out.write((char *)&t.normal.x,sizeof(float));
	out.write((char *)&t.normal.y,sizeof(float));
	out.write((char *)&t.normal.z,sizeof(float));
	
	//vertices
	out.write((char *)&t.a.x,sizeof(float));
    out.write((char *)&t.a.y,sizeof(float));
    out.write((char *)&t.a.z,sizeof(float));

	out.write((char *)&t.b.x,sizeof(float));
    out.write((char *)&t.b.y,sizeof(float));
    out.write((char *)&t.b.z,sizeof(float));

	out.write((char *)&t.c.x,sizeof(float));
    out.write((char *)&t.c.y,sizeof(float));
    out.write((char *)&t.c.z,sizeof(float));
  	out.write(endTag,2);
}

int width = 40;		//default width and length of model
int height = 40;
vector<float> hList;

string savefile = "stls/";

//Takes a height array height array of variable length and turns it into an STL file
void writeSTLfromArray(){
	out.open(savefile.c_str(),ios_base::binary);

	uint32_t triangleCount = (width-1)*(height-1)*2;	//number of facets in a void-free surface
	triangleCount += 4*(width-1);	//triangle counts for the walls of the model
	triangleCount += 4*(height-1);
	triangleCount += 2; 			//base triangles
	float planarScale = 40/width;

	if(out.good()){
		for(int i = 0; i < 80; i++){
			out.write("t",1);
		}
		//write a placeholder number
		out.write((char *)&triangleCount,4);
		for(int c = 1; c < width; c++){
			if((int)hList.at(c)>-50 & (int)hList.at(c-1)>-50 & (int)hList.at(c+width-1)>-50 ){
				vertex a = createVertex(c, 0,hList.at(c));
				vertex b = createVertex(c-1, 0,hList.at(c-1));
				vertex d = createVertex(c-1, 1,hList.at(c+width-1));
				
				vertex w = createVertex(c,0,0);				//used in model walls
				vertex z = createVertex(c-1,0,0);
				
				addTriangle(createTriangle(a,d,b));
				addTriangle(createTriangle(b,z,a));			//model walls
				addTriangle(createTriangle(w,a,z));
			}else{
				triangleCount-=3;
			}
		}
		for(int y = 1; y < height-1; y++){
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)>-50 & (int)hList.at((y-1)*width+x)>-50 & (int)hList.at(y*width+x-1)>-50 ){
					vertex a = createVertex(x,y,hList.at(y*width+x));
					vertex b = createVertex(x,y-1,hList.at((y-1)*width+x));
					vertex c = createVertex(x-1,y,hList.at(y*width+x-1));
					addTriangle(createTriangle(a,c,b));
				}else{
					triangleCount--;
				}
			}
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)>-50 & (int)hList.at(y*width+x-1)>-50 & (int)hList.at((y+1)*width+x-1)>-50 ){
					vertex a = createVertex(x,y,hList.at(y*width+x));		//same
					vertex b = createVertex(x-1,y,hList.at(y*width+x-1));
					vertex c = createVertex(x-1,y+1,hList.at((y+1)*width+x-1));
					addTriangle(createTriangle(a,c,b));
				}else{
					triangleCount--;
				}
			}
		}
		for(int x = 1; x < width; x++){
			if((int)hList.at((height-1)*width+x)>-50 & (int)hList.at((height-2)*width+x)>-50 & (int)hList.at((height-1)*width+x-1)>-50){
				vertex a = createVertex(x,height-1,hList.at((height-1)*width+x));		//same
				vertex b = createVertex(x,height-2,hList.at((height-2)*width+x));
				vertex c = createVertex(x-1,height-1,hList.at((height-1)*width+x-1));
				
				vertex w = createVertex(x,height-1,0);		//used in model walls
				vertex z = createVertex(x-1,height-1,0);
				
				addTriangle(createTriangle(a,c,b));
				addTriangle(createTriangle(c,a,z));			//model walls
				addTriangle(createTriangle(w,z,a));
			}else{
				triangleCount-=3;
			}
		}
		
		vertex st;
		vertex sb;
		vertex bt;
		vertex bb;
		for(int y = 1; y < width; y++){						//adds walls in the y direction for
			if((int)hList.at(y*width)>-50 & (int)hList.at((y-1)*width)>-50){
				st = createVertex(0,y,hList.at(y*width));			//for x=0 first
				sb = createVertex(0,y-1,hList.at((y-1)*width));
				bt = createVertex(0,y,0);
				bb = createVertex(0,y-1,0);
				
				addTriangle(createTriangle(bb,sb,st));
				addTriangle(createTriangle(st,bt,bb));
			}else{
				triangleCount-=2;
			}
			if((int)hList.at(y*width+width-1)>-50 & (int)hList.at(y*width-1)>-50){
				st = createVertex(width-1,y,hList.at(y*width+width-1));		//for x=width next
				sb = createVertex(width-1,y-1,hList.at(y*width-1));
				bt = createVertex(width-1,y,0);
				bb = createVertex(width-1,y-1,0);
				
				addTriangle(createTriangle(sb,bb,st));
				addTriangle(createTriangle(bt,st,bb));
			}else{
				triangleCount-=2;
			}
		}
		
		vertex origin = createVertex(0,0,0);					//create bottom surface
		vertex bottomright = createVertex(width-1,0,0);
		vertex topleft = createVertex(0,height-1,0);
		vertex topright = createVertex(width-1,height-1,0);
		addTriangle(createTriangle(origin,topright,bottomright));
		addTriangle(createTriangle(origin,topleft,topright));
		//triangleCount-=2;
		
		out.seekp(80);
		out.write((char *)&triangleCount,4);
	}
	cout << triangleCount << "\n";
}

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
	float verticalscale = 23.2;			//true_verticalscale gives models that are too flat to be interesting
	
	lat = atof(argv[1]);					//Latitude of NW corner
	printf("Using latitude: %f\n",lat);
	lng = atof(argv[2]);					//Longitude of NW corner
	printf("Using longitude: %f\n",lng);
	res = atoi(argv[3]);					//arcseconds/tick in model
	if(res%3!=0){							//must be a multiple of 3
		printf("Bad resolution\n");
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
	printf("Stepsize: %i\n",stepSize);
	printf("Using resolution %i\n",res);	//Arc
	height = 40*res/stepSize;
	width = 40*res/stepSize;

	hList.resize(width*height,0);

	savefile.append(std::string(argv[4]));
	printf("Saving to '%s'\n",savefile.c_str());

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
		printf("Extra tile in y");
		tilesOffsetY = (1200-i);
		getTile(lat-1.0,lng,2);
	}
	if(j+width*stepSize>1200){
		printf("Extra tile in x");
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
				//printf("%s",tiles[whichTile].c_str());
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
			
			//If a void exists, marks it as -100
			if(h<-100){
				h=-verticalscale*100*stepSize;
			}
			//rotate model to correct orientation
			//hList.at((height-1-y)*width+x) = h/(verticalscale*res); //cast verticalscale to int for COOl effect!
			hList.at((height-1-y/stepSize)*width+x/stepSize) = h/(verticalscale*stepSize)+baseHeight; 	//+baseHeight so that the bottom of the model does not bleed through to the top
		}
	}
	
	writeSTLfromArray();
	return 0;
}


