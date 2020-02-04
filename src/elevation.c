//Elevation.cpp
//Gets elevations at points and in regions

#include <stdio.h>

int getTile(char * filename, size_t len, float lat, float lng){
	char ns = lat>=0 ? "N" : "S"; //Positive is north
	char ew = lng>=0 ? "E" : "W";	//Positive is east

	int tileLat = abs( (int)floor(lat) );
	int tileLng = abs( (int)floor(lng) );

	return sprintf(filename, len, "./hgt_files/%c%02d%c%03d.hgt", ns, tileLat, ew, tileLng);
}

//returns the 'index' of the tile that contains a certain coordinate
//quicker than getTile()
int getTileIndex(float lat, float lng){
	return (int)floor(lat)*360 + (int)floor(lng);
}

int tileNumber = 0;
char tileName[100];
FILE * file;

//width and heigth are in 'pixels'
//heights must be able to accomodate width-many values
int getElevationLine(float* heights, int width, int nthLine, float startLat,float startLng, float vscale,float rot, int waterDrop, int baseHeight, int stepSize){
	int h;
	char number [2];

	int t = getTileIndex(_lat,_lng);

	//let's go with mercator space for now (Terrain2STL style)
	int y = nthLine;
	for(int x = 0; x<width; x++){
		float u = (float)y/1200 - (float)height/1200;
		float v = (float)x/1200;

		u *= stepSize;
		v *= stepSize;

		//get the lat and lng for each point
		float lat = startLat + u*cos(rot) + v*sin(rot);
		float lng = startLng + v*cos(rot) - u*sin(rot);

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
					getTile(tileName, intlat, intlng);

					close(file);
					#warning "Handle the case where we can't open the file - return zeros for ocean elev"
		      file = fopen(tilename, "r");
				}

				int p = (int) (1201*(intlng-floor(intlng)));		//x or lng component
				p += (int)(1201*(ceil(intlat)-intlat))* 1201;	//y or lat component

				fseek(file, p*2, SEEK_SET)
				fread(number, 1, 2, file);
		    h = number[1];
		    if(h<0){
		      h = h+255;
		    }
		    h += number[0]<<8;

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

		//not sure about this...
		float intHeight = westLng*(1-fracLng) + eastLng*fracLng;

		heights[x] = intHeight*vscale+baseHeight;
	}
	close(file);
	printf("Done getting elevations\n");
	return 1;
}


/*
//width and heigth are in 'pixels'
vector<float> getElevations(float _lat,float _lng,int width,int height,float vscale,float rot, int waterDrop, int baseHeight, int stepSize){
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

			u *= stepSize;
			v *= stepSize;

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
*/
