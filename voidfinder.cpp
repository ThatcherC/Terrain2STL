#include <iostream>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include <string>

// g++ voidfinder.cpp -o voidfinder -std=c++0x

using namespace std;

int main(int argc, char **argv){
  int h;
  char number [2];
	ifstream file;
  int count = 1;

  //open file

  file.open(argv[argc-1]);

  if(argv[1] == string("-c")){
    count = atoi(argv[2]);
  }

  //scan through every point and find voids
  //voids occur at -32768
  for(int i = 0; i < 2884802; i+=2){  //size of HGT file
    file.read(number,2);
    h = number[1];
    if(h<0){
      h = h+255;
    }
    h+= number[0]*256;

    //if there's even one void, print the filename and break
    if(h == -32768){
      count--;
      if(count==0){
        cout << argv[argc-1] << "\n";
        break;
      }
    }
  }

  return 0;
}
