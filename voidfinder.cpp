#include <iostream>
#include <stdint.h>
#include <fstream>
#include <math.h>
#include "stdio.h"
#include "stdlib.h"
#include <string>

// g++ elevstl.cpp -o elevstl -std=c++0x

using namespace std;

int main(int argc, char **argv){
  int h;
  char number [2];
	ifstream file;
  //open file
  cout << argv[0] << "\n";
  cout << argv[1] << "\n";
  file.open(argv[1]);


  //scan through every point and find voids
  //voids occur at -32768
  int count = 0;
  for(int i = 0; i < 2884802; i+=2){  //size of HGT file
    file.read(number,2);
    h = number[1];
    if(h<0){
      h = h+255;
    }
    h+= number[0]*256;
    if(h == -32768){
      count++;
    }
  }

  //count them and print
  cout << count << "\n";
  return 0;
}
