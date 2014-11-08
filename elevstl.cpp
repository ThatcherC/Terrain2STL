#include <iostream>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include <string>


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

//placeholder data
//It's the height array of Rockport Harbor
float hList [] = {18,15,15,15,17,20,22,26,31,36,41,47,49,35,25,23,25,29,33,38,39,37,37,39,42,45,47,48,48,49,50,51,51,51,51,51,51,51,51,50,22,18,16,15,15,15,17,20,24,30,33,42,47,38,27,23,21,25,30,34,36,34,34,35,38,42,44,46,47,48,49,50,50,51,50,50,50,50,49,48,24,18,16,15,15,15,15,16,19,23,26,35,40,33,26,23,18,21,24,28,32,32,32,34,36,38,42,44,46,48,49,49,49,50,50,50,49,49,48,47,24,20,17,15,15,15,15,15,17,17,21,29,33,29,24,21,18,19,21,24,29,30,31,30,33,35,39,43,47,48,48,48,49,50,49,49,49,49,48,47,31,28,29,21,17,16,17,16,15,15,16,26,31,27,22,21,18,17,20,22,25,29,30,29,34,37,40,43,47,48,48,49,50,49,49,48,48,48,47,45,35,33,34,29,22,20,19,18,18,18,12,19,31,28,22,21,18,16,18,21,23,26,28,29,34,40,44,47,48,49,49,51,52,49,47,46,44,43,41,37,37,37,40,35,27,24,22,22,23,22,14,13,30,34,25,21,19,16,18,20,22,24,25,30,35,38,43,46,48,49,50,53,57,49,42,36,31,28,26,23,40,41,44,41,32,26,23,23,24,21,13,12,29,40,29,21,18,13,15,18,20,22,24,28,32,36,40,43,46,48,49,52,50,39,26,16,12,11,9,5,43,46,48,46,37,29,26,25,24,18,6,15,31,41,31,20,16,9,14,18,16,18,21,23,28,32,37,40,43,46,46,45,34,19,6,1,1,1,1,1,48,52,52,49,43,34,30,28,26,19,5,14,28,38,27,17,14,6,11,15,12,15,17,20,25,30,34,36,39,41,36,25,10,2,1,0,0,0,0,0,51,54,54,52,47,39,35,32,29,25,9,7,20,29,21,12,9,2,3,6,6,10,11,17,24,29,30,32,32,26,15,4,1,0,0,0,0,0,0,0,53,54,54,54,51,44,40,38,35,35,22,8,10,17,10,2,1,1,1,0,1,2,4,15,22,25,21,17,12,3,1,1,0,0,0,0,0,0,0,0,53,53,52,53,53,46,43,43,41,40,33,22,8,4,1,1,0,0,0,0,0,0,1,9,15,14,6,1,0,0,0,0,0,0,0,0,0,0,0,0,52,50,48,49,51,46,45,43,42,41,36,31,15,1,1,0,0,0,0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,50,48,46,45,46,45,44,42,41,39,35,30,16,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,48,46,42,41,42,44,43,42,39,36,33,28,16,2,1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,44,42,39,35,38,41,42,41,38,35,32,28,21,13,11,12,12,10,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,40,38,36,32,32,36,38,39,36,33,30,28,26,23,18,20,21,22,17,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,36,35,32,29,28,30,33,36,34,32,29,28,30,28,21,23,25,27,28,19,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,35,32,28,28,28,29,31,32,31,29,29,31,28,22,22,25,29,31,29,17,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,35,36,32,28,28,28,28,30,30,28,30,33,30,24,23,26,31,35,34,29,14,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,36,37,36,33,29,28,28,28,29,28,31,34,34,29,27,29,33,39,40,33,24,10,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,34,35,37,38,38,34,28,28,28,28,28,31,34,35,32,33,33,35,44,45,38,33,27,12,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,35,36,39,40,40,39,32,28,28,28,29,31,34,35,34,35,35,37,48,49,44,39,36,24,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,38,38,41,42,42,42,40,31,28,28,28,30,33,35,35,34,35,39,51,53,49,45,39,28,15,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,40,41,41,40,39,40,35,30,28,28,30,32,34,34,34,34,40,52,54,49,46,40,31,19,14,8,0,0,0,0,0,0,0,0,0,0,0,0,0,45,42,41,39,36,33,35,35,31,29,28,29,31,35,35,34,34,41,48,48,44,40,35,28,20,17,13,11,11,7,0,0,0,0,0,0,0,0,0,0,50,47,44,41,32,29,31,32,31,30,30,30,33,35,36,35,32,34,33,32,27,25,23,19,17,16,15,13,12,8,0,0,0,0,0,0,0,0,3,0,54,52,50,46,33,26,27,30,30,30,30,31,33,35,36,35,31,27,23,21,18,16,16,16,16,16,15,13,12,9,3,0,0,0,2,0,0,0,5,1,54,55,55,50,36,25,21,25,27,27,29,30,32,34,34,33,30,27,22,17,16,16,17,20,23,19,15,14,12,11,7,0,4,4,6,0,0,1,2,0,52,55,58,53,37,24,16,16,19,23,26,27,29,30,31,30,25,22,18,15,15,17,20,26,26,20,15,12,12,12,11,4,2,2,1,0,0,0,0,0,49,51,53,51,43,27,15,8,11,16,20,22,23,24,24,22,17,15,12,9,11,14,18,20,18,16,12,11,11,5,3,1,0,0,0,0,0,0,0,0,42,44,44,45,44,33,17,6,5,6,9,12,13,12,9,6,3,3,2,1,2,4,7,10,11,9,8,9,7,2,0,0,0,0,0,0,0,0,0,0,38,36,33,34,39,32,19,8,3,1,1,1,2,1,1,1,1,1,0,0,1,1,1,1,1,2,5,7,6,2,0,0,0,0,0,0,0,0,0,0,33,27,24,25,28,26,17,11,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,3,6,7,4,0,0,0,0,0,0,0,0,0,0,23,22,21,22,24,24,16,8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,7,9,7,5,0,0,0,0,0,0,0,0,0,15,18,19,21,22,23,13,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,4,7,5,3,0,0,0,0,0,0,0,0,0,3,11,17,15,9,5,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,1,3,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

string savefile = "stls/";

//Takes a height array of length 1600 and turns it 40x40 STL file
void writeSTLfromArray(){
	out.open(savefile.c_str(),ios_base::binary);

	uint32_t triangleCount = 3042;		//number of facets in a void-free model [ (width-1)*(height-1)*2 ]

	if(out.good()){
		for(int i = 0; i < 80; i++){
			out.write("t",1);
		}
		//write a placeholder number
		out.write((char *)&triangleCount,4);
		for(int c = 1; c < width; c++){
			if((int)hList[c]!=-100 & (int)hList[c-1]!=-100 & (int)hList[c+width-1]!=-100 ){
				vertex a = createVertex(c, 0,hList[c]);
				vertex b = createVertex(c-1, 0,hList[c-1]);
				vertex d = createVertex(c-1, 1,hList[c+width-1]);
				addTriangle(createTriangle(a,d,b));
			}else{
				triangleCount--;
			}
		}
		for(int y = 1; y < height-1; y++){
			for(int x = 1; x < width; x++){
				if((int)hList[y*width+x]!=-100 & (int)hList[(y-1)*width+x]!=-100 & (int)hList[y*width+x-1]!=-100 ){
					vertex a = createVertex(x,y,hList[y*width+x]);
					vertex b = createVertex(x,y-1,hList[(y-1)*width+x]);
					vertex c = createVertex(x-1,y,hList[y*width+x-1]);
					addTriangle(createTriangle(a,c,b));
				}else{
					triangleCount--;
				}
			}
			for(int x = 1; x < width; x++){
				if((int)hList[y*width+x]!=-100 & (int)hList[y*width+x-1]!=-100 & (int)hList[(y+1)*width+x-1]!=-100 ){
					vertex a = createVertex(x,y,hList[y*width+x]);		//same
					vertex b = createVertex(x-1,y,hList[y*width+x-1]);
					vertex c = createVertex(x-1,y+1,hList[(y+1)*width+x-1]);
					addTriangle(createTriangle(a,c,b));
				}else{
					triangleCount--;
				}
			}
		}
		for(int x = 1; x < width; x++){
			if((int)hList[(height-1)*width+x]!=-100 & (int)hList[(height-2)*width+x]!=-100 & (int)hList[(height-1)*width+x-1]!=-100 ){
				vertex a = createVertex(x,height-1,hList[(height-1)*width+x]);		//same
				vertex b = createVertex(x,height-2,hList[(height-2)*width+x]);
				vertex c = createVertex(x-1,height-1,hList[(height-1)*width+x-1]);
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
	
	if(i+40*res>1201){
		perror("Error in x");
	}
	if(j+40*res>1201){
		perror("Error in y");
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
		for(int y = 0; y < 40; y++){
			for(int x = 0; x < 40; x++){
				tile.seekg((point+x*res+y*1201*res)*2,ios::beg);
				tile.read(number,2);
				h = number[1];
				if(h<0){
					h = h+255;
				}
				h+= number[0]*256;
				//rotate model to correct orientation
				if(h<-11071){
					h=-verticalscale*res*100;
				}
				hList[(39-y)*40+x] = h/(verticalscale*res); //cast verticalscale to int for COOl effect!
			}
		}
	}
	tile.close();
	writeSTLfromArray();
	return 0;
}


