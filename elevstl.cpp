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
float longscale = 1.0;
bool vertical;
bool crust;
float minheight = 1e30;
;
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
	v.x = x * longscale;
	v.y = y;
	v.z = z;
	return v;
}

vertex rotateVertex(vertex v){
	if (!vertical)
		return v;
	vertex r;
	r.x = -v.z;
	r.y = v.y;
	r.z = v.x;
	return r;
}

//Creates a triangle and its normal vector from three vertices
triangle createTriangle(vertex _j, vertex _k, vertex _l){
	vertex j = rotateVertex(_j);
	vertex k = rotateVertex(_k);
	vertex l = rotateVertex(_l);
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
float thickness = 0;
vector<float> hList;

string savefile = "stls/";

float baseheight(float below)
{
	if (crust)
		return below - thickness;
	else if (thickness)
		return minheight - thickness;
	else
		return 0;
}

vertex createVertexBelow(vertex v)
{
	vertex r;
	r.x = v.x;
	r.y = v.y;
	r.z = baseheight(v.z);
	return r;
}

//Takes a height array height array of variable length and turns it into an STL file
void writeSTLfromArray(){
	out.open(savefile.c_str(),ios_base::binary);

	uint32_t triangleCount = (width-1)*(height-1)*2;	//number of facets in a void-free surface
	triangleCount += 4*(width-1);	//triangle counts for the walls of the model
	triangleCount += 4*(height-1);
	if (crust)
		triangleCount += (width-1)*(height-1)*2;	//For bottom surface
	else
		triangleCount += 2; 			//base triangles
	float planarScale = 40/width;

	if(out.good()){
		for(int i = 0; i < 80; i++){
			out.write("t",1);
		}
		//write a placeholder number
		out.write((char *)&triangleCount,4);
		for(int c = 1; c < width; c++){
			if((int)hList.at(c)!=-99 & (int)hList.at(c-1)!=-99 & (int)hList.at(c+width-1)!=-99 ){
				vertex a = createVertex(c, 0,hList.at(c));
				vertex b = createVertex(c-1, 0,hList.at(c-1));
				vertex d = createVertex(c-1, 1,hList.at(c+width-1));
				
				vertex w = createVertexBelow(a);			//used in model walls
				vertex z = createVertexBelow(b);
				
				addTriangle(createTriangle(a,d,b));
				addTriangle(createTriangle(b,z,a));			//model walls
				addTriangle(createTriangle(w,a,z));
			}else{
				triangleCount-=3;
			}
		}
		for(int y = 1; y < height-1; y++){
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)!=-99 & (int)hList.at((y-1)*width+x)!=-99 & (int)hList.at(y*width+x-1)!=-99 ){
					vertex a = createVertex(x,y,hList.at(y*width+x));
					vertex b = createVertex(x,y-1,hList.at((y-1)*width+x));
					vertex c = createVertex(x-1,y,hList.at(y*width+x-1));
					addTriangle(createTriangle(a,c,b));

					if(crust){
						vertex a1 = createVertexBelow(a);
						vertex b1 = createVertexBelow(b);
						vertex c1 = createVertexBelow(c);
						addTriangle(createTriangle(a1,b1,c1));
					}
				}else{
					triangleCount-=crust?2:1;
				}
			}
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)!=-99 & (int)hList.at(y*width+x-1)!=-99 & (int)hList.at((y+1)*width+x-1)!=-99 ){
					vertex a = createVertex(x,y,hList.at(y*width+x));		//same
					vertex b = createVertex(x-1,y,hList.at(y*width+x-1));
					vertex c = createVertex(x-1,y+1,hList.at((y+1)*width+x-1));
					addTriangle(createTriangle(a,c,b));

					if(crust){
						vertex a1 = createVertexBelow(a);
						vertex b1 = createVertexBelow(b);
						vertex c1 = createVertexBelow(c);
						addTriangle(createTriangle(a1,b1,c1));
					}
				}else{
					triangleCount-=crust?2:1;
				}
			}
		}
		for(int x = 1; x < width; x++){
			if((int)hList.at((height-1)*width+x)!=-99 & (int)hList.at((height-2)*width+x)!=-99 & (int)hList.at((height-1)*width+x-1)!=99 ){
				vertex a = createVertex(x,height-1,hList.at((height-1)*width+x));		//same
				vertex b = createVertex(x,height-2,hList.at((height-2)*width+x));
				vertex c = createVertex(x-1,height-1,hList.at((height-1)*width+x-1));
				
				vertex w = createVertexBelow(a);		//used in model walls
				vertex z = createVertexBelow(c);
				
				addTriangle(createTriangle(a,c,b));
				addTriangle(createTriangle(c,a,z));			//model walls
				addTriangle(createTriangle(w,z,a));
			}else{
				triangleCount-=3;
			}
		}
		
		for(int y = 1; y < height; y++){						//adds walls in the y direction for
			vertex st = createVertex(0,y,hList.at(y*width));		//for x=0 first
			vertex sb = createVertex(0,y-1,hList.at((y-1)*width));
			vertex bt = createVertexBelow(st);
			vertex bb = createVertexBelow(sb);
			
			addTriangle(createTriangle(bb,sb,st));
			addTriangle(createTriangle(st,bt,bb));
			
			st = createVertex(width-1,y,hList.at(y*width+width-1));		//for x=width next
			sb = createVertex(width-1,y-1,hList.at(y*width-1));
			bt = createVertexBelow(st);
			bb = createVertexBelow(sb);
			
			addTriangle(createTriangle(sb,bb,st));
			addTriangle(createTriangle(bt,st,bb));
		}

		if(!crust){
			vertex origin = createVertex(0,0,minheight);
			vertex bottomright = createVertex(width-1,0,minheight);
			vertex topleft = createVertex(0,height-1,minheight);
			vertex topright = createVertex(width-1,height-1,minheight);
			addTriangle(createTriangle(origin,topright,bottomright));
			addTriangle(createTriangle(origin,topleft,topright));
		}
		out.seekp(80);
		out.write((char *)&triangleCount,4);
	}
	cout << triangleCount << "\n";
}

float verticalscale = 23.2;			// true verticalscale 92.7 meters/arcsecond (at equator) gives models that are too flat to be interesting

const char ** checkOption(const char *arg, const char **argv)
{
	if (strcmp(arg, "--vertical")==0)
		vertical = true;
	else if (strcmp(arg, "--crust")==0)
		crust = true;
	else if (strcmp(arg, "--thickness")==0)
		thickness = atoi(*argv++);
	else if (strcmp(arg, "--scale")==0)
		verticalscale = atof(*argv++);
	else{
		printf("Unrecognised argument %s\n", arg);
		exit(0);
	}
	return argv; 
}

int main(int argc, const char **argv)			//lat, long, res, outfile
{
	string file;
	int point;
	float lat;
	float lng;
	int res;
	int tile_n;
	int tile_w;
	if(argc < 5){
		printf("Not enough arguents\n");
		exit(0);
	}
	int argno = 1;
	argv++; // not interested in argv[0]
	while(*argv){
		const char *arg = *argv++;
		if(arg[0]=='-' && arg[1]=='-')
			argv = checkOption(arg, argv);
		else switch(argno++){
		case 1:
			lat = atof(arg);					//Latitude of NW corner
			printf("Using latitude: %f\n",lat);
			break;
		case 2:
			lng = atof(arg);					//Longitude of NW corner
			printf("Using longitude: %f\n",lng);
			break;
		case 3:
			res = atoi(arg);					//arcseconds/tick in model
			if(res%3!=0){							//must be a multiple of 3
				printf("Bad resolution\n");
				return 0;
			}
			break;
		case 4:
			savefile.append(std::string(arg));
			printf("Saving to '%s'\n",savefile.c_str());
			break;
		default:
			printf("Too many arguments\n");
			exit(0);
		}
	}
	if (crust && !thickness)
		thickness = 2;
	res = res/3;							//SRTM data already has a resolution of 3 arcseconds
	printf("Using resolution %i\n",res);	//Arc

        // Mathematical expression: Length of a degree of longitude = cos(latitude) * 111.325 kilometers
        longscale = cos(lat*0.0174532925199433);   // ratio of long to lat
        printf ("Longitude scaling factor (at NW corner): %f\n", longscale);

	height = 40*res; 
	width = 40*res / longscale;
	hList.resize(width*height,0);
	
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
				if(h<-100){
					h=-verticalscale*100;
				}
				//rotate model to correct orientation
				//hList.at((height-1-y)*width+x) = h/(verticalscale*res); //cast verticalscale to int for COOl effect!
				hList.at((height-1-y)*width+x) = h/(verticalscale)+1;   //+1 so that the bottom of the model does not bleed through to the top
			}
		}
		for(int y = 0; y < height; y++){
			for(int x = 0; x < width; x++){
                                int index = (height-1-y)*width+x;
                                h = hList.at(index);
                                if (h == -99)
                                {
					// Ideally you would take average of surrounding 4 (or 8) non-void points
					// This actually only looks at previous x/y (which are already corrected).
					// Will go a bit wrong if you are unlucky enough to have a void when x and y is zero.
					if (x && y)
						h = (hList.at(index-1) + hList.at(index+width))/2;
					else if (x)
						h = hList.at(index-1);
					else if (y)
						h = hList.at(index+width);
					else{
						printf("Missing data at NW corner - aborting\n");
						exit(0);
					}
					hList.at(index) = h;
				}
				if (h < minheight)
					minheight = h;
			}
		}
	}
	tile.close();
	writeSTLfromArray();
	return 0;
}


