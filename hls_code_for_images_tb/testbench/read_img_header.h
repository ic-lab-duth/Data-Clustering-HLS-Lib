#ifndef __READ_IMG_HEADER__
#define __READ_IMG_HEADER__

#include "datatypes_spec.h"

#include <sys/resource.h>

using namespace std;
#include <iostream>
#include <fstream>
#include <vector>

#include "bmpUtil/bmp_io.hpp"

void read_image_data(string filename, int width, int height, P_TYPE * data) {
  

  unsigned long int imgWidth = width;
  long int imgHeight = height;
  
  static unsigned char *rarray = new unsigned char[imgWidth*imgHeight];
  static unsigned char *garray = new unsigned char[imgWidth*imgHeight];
  static unsigned char *barray = new unsigned char[imgWidth*imgHeight];


  bmp_read((char*)filename.c_str(), &imgWidth, &imgHeight, &rarray, &garray, &barray);
  
  P_TYPE tmp;
  for (int i=0; i<width*height; i++) {
    data[i].coord[0] = (COORD_TYPE)((int)rarray[i]);
    data[i].coord[1] = (COORD_TYPE)((int)garray[i]);
    data[i].coord[2] = (COORD_TYPE)((int)barray[i]);
    
    data[i].id = i;
  }

  delete [] rarray;
  delete [] garray;
  delete [] barray;

};


void write_image_from_data(string filename, int width, int height, P_TYPE * data, CENTER_T * center) {

  unsigned long int imgWidth = width;
  long int imgHeight = height;

  unsigned char *rarray = new unsigned char[imgWidth*imgHeight];
  unsigned char *garray = new unsigned char[imgWidth*imgHeight];
  unsigned char *barray = new unsigned char[imgWidth*imgHeight];

  int x;

  for (int i=0; i<N_POINTS; i++) {
    rarray[i] = (unsigned char)center[data[i].id_cluster].coord[0].to_int();
    garray[i] = (unsigned char)center[data[i].id_cluster].coord[1].to_int();
    barray[i] = (unsigned char)center[data[i].id_cluster].coord[2].to_int();
  }

  bmp_24_write((char*)filename.c_str(),imgWidth, imgHeight, rarray, garray, barray);

  delete [] rarray;
  delete [] garray;
  delete [] barray;
};

/*
void StackSize(){
	std::cout << "Program's stack size is exteded" << std::endl;
	const rlim_t kStackSize = 60*IMG_HGHT*IMG_WDTH;
    struct rlimit rl;
    int result;
    result = getrlimit(RLIMIT_STACK, &rl);
    if (result == 0) {
        if (rl.rlim_cur < kStackSize) {
            rl.rlim_cur = kStackSize;
            result = setrlimit(RLIMIT_STACK, &rl);
            if (result != 0) fprintf(stderr, "setrlimit returned result = %d\n", result);
        }
    }
};
*/

#endif
