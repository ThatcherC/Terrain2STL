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


  //scan through every point and find zeros
    //what's a zero exactly?

  file.read(number,2);
  h = number[1];
  if(h<0){
    h = h+255;
  }
  h+= number[0]*256;
  cout << h << "\n";

  //count them and print
  return 0;
}
