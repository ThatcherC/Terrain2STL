//STL.cpp
//makes an stl file from a big array
#include <iostream>
#include <vector>
//#include <stdint.h>
//#include <fstream>
#include <cmath>
//#include "stdio.h"
//#include "stdlib.h"
#include <string>
#include "Vector.h"
#include "STLWriter.h"

using namespace std;

struct triangle{
  Vector a;
  Vector b;
  Vector c;
  Vector normal;
};


int voidCutoff = -1900;
char endTag[2] = {0,0};

//Determines the normal vector of a triangle from three vertices
Vector normalOf(const Vector &p1, const Vector &p2, const Vector &p3){
	Vector u(0,0,0);
	Vector v(0,0,0);
	Vector r(0,0,0);
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

//Creates a triangle and its normal vector from three vertices
triangle createTriangle(const Vector &j, const Vector &k, const Vector &l){
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
	cout.write((char *)&t.normal.x,sizeof(float));
	cout.write((char *)&t.normal.y,sizeof(float));
	cout.write((char *)&t.normal.z,sizeof(float));

	//vertices
	cout.write((char *)&t.a.x,sizeof(float));
    cout.write((char *)&t.a.y,sizeof(float));
    cout.write((char *)&t.a.z,sizeof(float));

	cout.write((char *)&t.b.x,sizeof(float));
    cout.write((char *)&t.b.y,sizeof(float));
    cout.write((char *)&t.b.z,sizeof(float));

	cout.write((char *)&t.c.x,sizeof(float));
    cout.write((char *)&t.c.y,sizeof(float));
    cout.write((char *)&t.c.z,sizeof(float));
  	cout.write(endTag,2);
}

//Takes a height array of variable length and turns it into an STL file
void writeSTLfromArray(const vector<float> &hList, int width, int height, float xScale){
	uint32_t triangleCount = (width-1)*(height-1)*2;	//number of facets in a void-free surface
	triangleCount += 4*(width-1);	//triangle counts for the walls of the model
	triangleCount += 4*(height-1);
	triangleCount += 2; 			//base triangles
	float planarScale = 40/width;

  //Depending on implementation and whether or not geodesic selection boxes
  //are used, this might be important:
  //float xScale = (float)cos(globalLat);

	if(cout.good()){
		for(int i = 0; i < 80; i++){
			cout.write("t",1);
		}
		//write a placeholder number
		cout.write((char *)&triangleCount,4);
		for(int c = 1; c < width; c++){
			if((int)hList.at(c)>voidCutoff & (int)hList.at(c-1)>voidCutoff & (int)hList.at(c+width-1)>voidCutoff ){
				Vector a(c*xScale, 0,hList.at(c));
				Vector b((c-1)*xScale, 0,hList.at(c-1));
				Vector d((c-1)*xScale, 1,hList.at(c+width-1));

				Vector w(c*xScale,0,0);				//used in model walls
				Vector z((c-1)*xScale,0,0);

				addTriangle(createTriangle(a,d,b));
				addTriangle(createTriangle(b,z,a));			//model walls
				addTriangle(createTriangle(w,a,z));
			}else{
				triangleCount-=3;
			}
		}
		for(int y = 1; y < height-1; y++){
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)>voidCutoff & (int)hList.at((y-1)*width+x)>voidCutoff & (int)hList.at(y*width+x-1)>voidCutoff ){
					Vector a(x*xScale,y,hList.at(y*width+x));
					Vector b(x*xScale,y-1,hList.at((y-1)*width+x));
					Vector c((x-1)*xScale,y,hList.at(y*width+x-1));
					addTriangle(createTriangle(a,c,b));
				}else{
					triangleCount--;
				}
			}
			for(int x = 1; x < width; x++){
				if((int)hList.at(y*width+x)>voidCutoff & (int)hList.at(y*width+x-1)>voidCutoff & (int)hList.at((y+1)*width+x-1)>voidCutoff ){
					Vector a = Vector(x*xScale,y,hList.at(y*width+x));		//same
					Vector b = Vector((x-1)*xScale,y,hList.at(y*width+x-1));
					Vector c = Vector((x-1)*xScale,y+1,hList.at((y+1)*width+x-1));
					addTriangle(createTriangle(a,c,b));
				}else{
					triangleCount--;
				}
			}
		}
		for(int x = 1; x < width; x++){
			if((int)hList.at((height-1)*width+x)>voidCutoff & (int)hList.at((height-2)*width+x)>voidCutoff & (int)hList.at((height-1)*width+x-1)>voidCutoff){
				Vector a = Vector(x*xScale,height-1,hList.at((height-1)*width+x));		//same
				Vector b = Vector(x*xScale,height-2,hList.at((height-2)*width+x));
				Vector c = Vector((x-1)*xScale,height-1,hList.at((height-1)*width+x-1));

				Vector w = Vector(x*xScale,height-1,0);		//used in model walls
				Vector z = Vector((x-1)*xScale,height-1,0);

				addTriangle(createTriangle(a,c,b));
				addTriangle(createTriangle(c,a,z));			//model walls
				addTriangle(createTriangle(w,z,a));
			}else{
				triangleCount-=3;
			}
		}

		Vector st = Vector(0,0,0);
		Vector sb = Vector(0,0,0);
		Vector bt = Vector(0,0,0);
		Vector bb = Vector(0,0,0);
		for(int y = 1; y < width; y++){						//adds walls in the y direction for
			if((int)hList.at(y*width)>voidCutoff & (int)hList.at((y-1)*width)>voidCutoff){
				st =  Vector(0,y,hList.at(y*width));			//for x=0 first
				sb =  Vector(0,y-1,hList.at((y-1)*width));
				bt =  Vector(0,y,0);
				bb =  Vector(0,y-1,0);

				addTriangle(createTriangle(bb,sb,st));
				addTriangle(createTriangle(st,bt,bb));
			}else{
				triangleCount-=2;
			}
			if((int)hList.at(y*width+width-1)>voidCutoff & (int)hList.at(y*width-1)>voidCutoff){
				st =  Vector((width-1)*xScale,y,hList.at(y*width+width-1));		//for x=width next
				sb = Vector((width-1)*xScale,y-1,hList.at(y*width-1));
				bt = Vector((width-1)*xScale,y,0);
				bb = Vector((width-1)*xScale,y-1,0);

				addTriangle(createTriangle(sb,bb,st));
				addTriangle(createTriangle(bt,st,bb));
			}else{
				triangleCount-=2;
			}
		}

		Vector origin = Vector(0,0,0);					//create bottom surface
		Vector bottomright = Vector((width-1)*xScale,0,0);
		Vector topleft = Vector(0,height-1,0);
		Vector topright = Vector((width-1)*xScale,height-1,0);
		addTriangle(createTriangle(origin,topright,bottomright));
		addTriangle(createTriangle(origin,topleft,topright));
		//triangleCount-=2;

		cout.seekp(80);
		cout.write((char *)&triangleCount,4);
	}
	clog << "Triangle count: " << triangleCount << "\n";
}
