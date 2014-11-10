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

	uint32_t triangleCount = (width-1)*(height-1)*2;	//number of facets in a void-free model
	float planarScale = 40/width;

	if(out.good()){
		for(int i = 0; i < 80; i++){
			out.write("t",1);
		}
		//write a placeholder number
		out.write((char *)&triangleCount,4);
		for(int c = 1; c < width; c++){
			if((int)hList.at(c)!=-100 & (int)hList.at(c-1)!=-100 & (int)hList.at(c+width-1)!=-100 ){
				vertex a = createVertex(c, 0,hList.at(c));
				vertex b = createVertex(c-1, 0,hList.at(c-1));
				vertex d = createVertex(c-1, 1,hList.at(c+width-1));
				addTriangle(createTriangle(a,d,b));
			}else{
				triangleCount--;
			}
		}
		for(int y = 1; y < height-1; y++){
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)!=-100 & (int)hList.at((y-1)*width+x)!=-100 & (int)hList.at(y*width+x-1)!=-100 ){
					vertex a = createVertex(x,y,hList.at(y*width+x));
					vertex b = createVertex(x,y-1,hList.at((y-1)*width+x));
					vertex c = createVertex(x-1,y,hList.at(y*width+x-1));
					addTriangle(createTriangle(a,c,b));
				}else{
					triangleCount--;
				}
			}
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)!=-100 & (int)hList.at(y*width+x-1)!=-100 & (int)hList.at((y+1)*width+x-1)!=-100 ){
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
			if((int)hList.at((height-1)*width+x)!=-100 & (int)hList.at((height-2)*width+x)!=-100 & (int)hList.at((height-1)*width+x-1)!=-100 ){
				vertex a = createVertex(x,height-1,hList.at((height-1)*width+x));		//same
				vertex b = createVertex(x,height-2,hList.at((height-2)*width+x));
				vertex c = createVertex(x-1,height-1,hList.at((height-1)*width+x-1));
				addTriangle(createTriangle(a,c,b));
			}else{
				triangleCount--;
			}
		}
		out.seekp(80);
		out.write((char *)&triangleCount,4);
	}
	cout << triangleCount << "\n";
}

int main(int argc, char **argv)			//lat, long, res
{
	string file;
	int point;
	float lat;
	float lng;
	int res;
	int tile_n;
	int tile_w;
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
	printf("Using resolution %i\n",res);	//Arc
	height = 40*res;
	width = 40*res;
	hList.resize(width*height,0);
	
	savefile.append(std::string(argv[4]));
	printf("Saving to '%s'\n",savefile.c_str());
	
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
	puts(file.c_str());
	
	//-------Find starting file index---------------
	float n = (lat-floor(lat))*3600;
	float e = (lng-floor(lng))*3600;
	
	int i = 1201-(int)(n/(3));
	int j = (int)(e/(3));
	
	if(i+height>1201){
		perror("Error in y");
	}
	if(j+width>1201){
		perror("Error in x");
	}
	
	point = j+i*1201;						//the file index of the NW corner

	//------------Open file and read data into array----------------------------
	ifstream tile;
	tile.open(file.c_str(),ios::in|ios::binary);
	int h;
	if (!tile.is_open()){
		perror ("Error opening file");
	}else{
		char number [2];
		for(int y = 0; y < height; y++){
			for(int x = 0; x < width; x++){
				tile.seekg((point+x+y*1201)*2,ios::beg);
				tile.read(number,2);
				h = number[1];
				if(h<0){
					h = h+255;
				}
				h+= number[0]*256;
				//If a void exists, marks it as -100
				if(h<-1171){
					h=-verticalscale*res*100;
				}
				//rotate model to correct orientation
				//hList.at((height-1-y)*width+x) = h/(verticalscale*res); //cast verticalscale to int for COOl effect!
				hList.at((height-1-y)*width+x) = h;
			}
		}
	}
	tile.close();
	writeSTLfromArray();
	return 0;
}


