#include "../KMeans_IO_static.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

#include <mc_scverify.h>


/* 
* HELPER FUNCTIONS
*/

void create_random_data(std::vector<std::pair<COORD_TYPE, COORD_TYPE>> &km_dt_data) {
  for (int i=0; i< N_POINTS; i++) {
    COORD_TYPE x ,y;
    int mulx, muly;

    mulx = rand()%60000;
    x = 5.1 * mulx;

    muly = rand()%50000;
    y = 4.3 * muly;

    km_dt_data.push_back(std::make_pair(x,y));
  }
};


/*
*  MAIN FUNCTION TESTBENCH
*/

CCS_MAIN(int argc, char *argv[]) {
  std::string filename = "./../clustering_data/data_1.txt";
  std::string method = "";
  

  int number_of_tests = 1;

  for (int test = 0; test<number_of_tests; test++) {

    // CREATE RANDOM DATA 
    std::vector<std::pair<COORD_TYPE, COORD_TYPE>> data_points;
    create_random_data(data_points);
    
    /*
    * HLS VERSION
    */
    C_TYPE hls_clusters_centers[K_CLSTRS];
    P_TYPE points_mem[N_POINTS];

    KMEANS<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_io;
    

    int id = 0;
    for (auto pnt : data_points) {
      points_mem[id].coord[0] = pnt.first;
      points_mem[id].coord[1] = pnt.second;
      id++;
    }

    km_io.run(points_mem, hls_clusters_centers);

  }
    

  CCS_RETURN(0);
}

