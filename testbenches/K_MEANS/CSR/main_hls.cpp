#include "../../../hls_code/K-MEANS/KMeans_nC_SD_CSR.h"

#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <mc_scverify.h>


template<typename T, int S, int N>
int CSR_conv(std::array<T,N> &x_data, std::array<T,N> &y_data, std::array<T,N> &crd, std::vector<CSR_POS_T> &pos1) {
    // INIT
  std::array<int,S> offset;
  ac::init_array<AC_VAL_0>(&offset[0], S);
  
  // FIRST MEMORY READ
  for (int i=0; i<N; i++) {
    offset[y_data[i].to_int()] ++;
  }
  
  std::vector<CSR_POS_T> pos;

  // CREATE POS
  CSR_POS_T curr;
  curr.index = 0;
  curr.pointer = 0;

  pos.push_back(curr);
  pos[0].index = 0;
  int id = 0;
  for (int i=0; i<S-1; i++) {
    if (offset[i] > 0) {

      pos.back().pointer = (COORD_TYPE)i;

      curr.index = pos.back().index + offset[i];
      curr.pointer = 0;
         
      pos.push_back(curr);

      id++;
    }
  }

  int size_of_pos = id+1;

  // SECOND MEMORY READ - CREATE CRD
  for (int i=0; i<N; i++) {
    int pos_indx = y_data[i].to_int();
    int cur_indx;
    for (int j=0; j<size_of_pos-1; j++) {
      if(pos[j].pointer == (COORD_TYPE)pos_indx) {
        cur_indx = j;
        break;
      }
    }
    int indx =  pos[cur_indx].index + offset[pos_indx] - 1;
    crd[indx] = x_data[i];
    offset[pos_indx]--;
  }
  
  pos1 = pos;
  return size_of_pos;

}


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
  

  int number_of_tests = 1;
  for (int test = 0; test<number_of_tests; test++) {

    // CREATE RANDOM DATA 
    std::vector<std::vector<COORD_TYPE>> data_points;
    //create_random_data(data_points);
    read_data(data_points, "../data_500_16.csv");

    // ~~~~~~~ CSR ~~~~~~~
    std::array<COORD_TYPE,N_POINTS> x_data, y_data, crd;
    std::vector<CSR_POS_T> pos;
    C_TYPE hls_clusters_centers_sd[K_CLSTRS];
    
    for (int i=0; i<N_POINTS; i++) {
      std::vector<COORD_TYPE> pnt = data_points[i];
      x_data[i] = pnt[0];
      y_data[i] = pnt[1];
      if (i<K_CLSTRS) {
        hls_clusters_centers_sd[i][0] = pnt[0];
        hls_clusters_centers_sd[i][1] = pnt[1];
      }
    }

    int xx = CSR_conv<COORD_TYPE,MAX_VALUE,N_POINTS>(x_data, y_data, crd, pos);
    std::cout << " POS SIZE = " << xx << std::endl;
    
    std::array<CSR_POS_T, PS> a_pos;
    
    int id = 0;
    for (auto p : pos) {
      a_pos[id] = p;
      id++;
    }

    CSR_CRD_T mem[N_POINTS];
    for (int i=0; i<N_POINTS; i++) {
      mem[i].coord[0] = crd[i];
      mem[i].id_cluster = 0;
    }
    
    KMEANS_SD<K_CLSTRS, N_POINTS, DATA_DIM, PS, M_ITERAT> km_sd;
    km_sd.run(mem, &a_pos[0], hls_clusters_centers_sd);
    
    for (int i=0; i<K_CLSTRS; i++) {
      std::cout << hls_clusters_centers_sd[i][0] << ", " << hls_clusters_centers_sd[i][1] << std::endl;
    }

  }
  
  CCS_RETURN(0);
}
