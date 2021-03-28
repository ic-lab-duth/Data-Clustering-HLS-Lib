#include "../../hls_code/K-MEANS/KMeans.h"
// #include "../../hls_code/K-MEANS/KMeans_nC.h"
// #include "../../hls_code/K-MEANS/KMeans_nC_TI_Elkan_HW.h" 
// #include "../../hls_code/K-MEANS/KMeans_nC_TI_Sums.h" 


#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>
#include <fstream>
#include <string>

#include <mc_scverify.h>

/* 
* HELPER FUNCTIONS
*/

void read_data(std::vector<std::vector<COORD_TYPE>> &data, std::string filename) {
  
  std::ifstream myFile(filename);
  
  if (!myFile.is_open()) throw std::runtime_error("Could not open file!");
  
  std::string line, colname;
  int val;
  
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
      tmp_point.push_back(std::stoi(val));
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


/*
*  MAIN FUNCTION TESTBENCH
*/
CCS_MAIN(int argc, char *argv[]) {

  /*
  *  SELECT THE CORRECT CLASS
  * 
  *  {CLASS}<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km;
  * 
  *  Available classes:
  *  1. K-Means with cost calculation                : KMEANS
  *  2. K-Means without cost calculation             : KMEANS_NC
  *  3. K-Means without cost + Elkan's Triangle Ineq : KMEANS_ELKAN
  *  4. K-Means without cost + Elkan's TI using Sums : KMEANS_SUMS     
  */
  KMEANS_NC<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km;

  int number_of_tests = 1;
  for (int test = 0; test<number_of_tests; test++) {

    // CREATE RANDOM DATA 
    std::vector<std::vector<COORD_TYPE>> data_points;
    //create_random_data(data_points);
    read_data(data_points, "../datasets/data_8000_8.csv");

    P_TYPE points_mem[N_POINTS];
    C_TYPE clusters[K_CLSTRS]; 

    for (int i=0; i<N_POINTS; i++) {
      points_mem[i].id_cluster = 0;
      for (int j=0; j<DATA_DIM; j++) {
        points_mem[i].coord[j] = data_points[i][j];
      }
    }
    
    km.run(points_mem, clusters);
    
    for (int i=0;i<K_CLSTRS; i++) {
      std::cout << clusters[i][0] << ", " << clusters[i][1] << std::endl;
    }
  }
  
  
  
  CCS_RETURN(0);
}


