#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string.h>
#include <assert.h>
#include <string>
#include <algorithm>
#include <iomanip>

#include "../../hls_code/DBSCAN/DBScan.h"
#include "../../hls_code/datatypes_spec.h"
#include <mc_scverify.h>


void read_data(std::vector<std::vector<COORD_TYPE>> &data, std::string filename) {
  
  std::ifstream myFile(filename);
  
  if (!myFile.is_open()) throw std::runtime_error("Could not open file!");
  
  std::string line, colname;
  float val;
  
  while(std::getline(myFile, line)) {
    std::vector<COORD_TYPE> tmp_point;
    std::vector<std::string> vec;
    
    std::string token;
    for (unsigned i=0; i<line.size(); i++) {
      char current = line[i];
      if (current == ';') {
        vec.push_back(token);
        token.clear();
      } else {
        token.push_back(current);
      }
    }
    
    vec.push_back(token);
    
    for (auto val:vec) {
      tmp_point.push_back(std::stof(val));
    }
    data.push_back(tmp_point);
  }
  
  myFile.close();
  
}


void create_random_data(std::vector<std::vector<COORD_TYPE>> &data) {
  for (int i=0; i< N_POINTS; i++) {
    
    std::vector<COORD_TYPE> point;
    
    for (int j=0; j<DATA_DIM; j++) {

      COORD_TYPE x;
      int mulx;
      
      int s = rand()%100;
      if (s>0)
        mulx = rand()%MAX_VALUE;
      else 
        mulx = -rand()%MAX_VALUE;

      x = (COORD_TYPE)mulx;
      point.push_back(x);

    }

    data.push_back(point);
  }
};

CCS_MAIN(int argc, char *argv[]) {
    
    int minPts = 4;
    ac_fixed<10,1,false> eps = 0.0064;

    // HLS    
    std::vector<std::vector<COORD_TYPE>> data_points;
    read_data(data_points, "../data_6.csv");

    P_TYPE points_mem[N_POINTS];
    C_TYPE clusters_centers[N_POINTS];
    bool valid_centers[N_POINTS];
    
    for (int i=0; i<N_POINTS; i++) {
      std::vector<COORD_TYPE> pnt = data_points[i];
      points_mem[i].id_cluster = 0;
      for (int j=0; j<DATA_DIM; j++) {
        points_mem[i].coord[j] = data_points[i][j]/3;
        //std::cout << points_mem[i].coord[j] << std::endl;
      }  
    }
    
    bool pnt_type[N_POINTS];
    PID_TYPE indexes[N_POINTS];
    

    DBSCAN<N_POINTS, DATA_DIM> dbs;
    dbs.run(points_mem, pnt_type, indexes, minPts, eps);

    CCS_RETURN(0);
}
