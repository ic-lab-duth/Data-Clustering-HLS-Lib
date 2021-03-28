#include "../../../hls_code/K-MEANS/KMeans.h"
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


void read_data(std::vector<std::vector<COORD_TYPE>> &km_dt_data, std::string filename) {
  std::ifstream myFile(filename);

  // Make sure the file is open
  if(!myFile.is_open()) throw std::runtime_error("Could not open file");

  // Helper vars
  std::string line, colname;
  int val;

  // Read data, line by line
  while(std::getline(myFile, line))
  {
    std::vector<COORD_TYPE> tmp_point;
    // Create a stringstream of the current line
    std::stringstream ss(line);
    
    // Keep track of the current column index
    int colIdx = 0;
    
    // Extract each integer
    while(ss >> val){

        // Add the current integer to the 'colIdx' column's values vector
        tmp_point.push_back(val);
        
        // If the next token is a comma, ignore it and move on
        if(ss.peek() == ';') ss.ignore();
        
        // Increment the column index
        colIdx++;
    }
    km_dt_data.push_back(tmp_point);
  }

    // Close file
    myFile.close();

}
/*
*  MAIN FUNCTION TESTBENCH
*/

CCS_MAIN(int argc, char *argv[]) {
  std::string filename = "./../clustering_data/data_1.txt";
  std::string method = "";
  

  int number_of_tests = 1;

  for (int test = 0; test<number_of_tests; test++) {

    // CREATE RANDOM DATA 
    std::vector<std::vector<COORD_TYPE>> data_points;
    // create_random_data(data_points);
    read_data(data_points, "data.csv");

    C_TYPE hls_clusters_centers_sd[K_CLSTRS];
    C_TYPE hls_clusters_centers[K_CLSTRS];

    for (int i=0; i<K_CLSTRS; i++) {
      COORD_TYPE x = (COORD_TYPE)(rand() % MAX_VALUE);
      COORD_TYPE y = (COORD_TYPE)(rand() % MAX_VALUE);

      hls_clusters_centers[i][0] = x;
      hls_clusters_centers[i][1] = y;

      hls_clusters_centers_sd[i][0] = x;
      hls_clusters_centers_sd[i][1] = y;
    }

    /*
    * HLS VERSION
    */
    P_TYPE points_mem[N_POINTS];
    // C_TYPE hls_clusters_centers[K_CLSTRS];
    
    for (int i=0; i<N_POINTS; i++) {
      std::vector<COORD_TYPE> pnt = data_points[i];
      for (int j=0; j<DATA_DIM; j++) {
        points_mem[i].coord[j] = pnt[j];
      }  
    }
    // std::cout << points_mem[0].coord[0] << ", " << points_mem[0].coord[1] << std::endl;
    // std::cout << points_mem[1].coord[0] << ", " << points_mem[1].coord[1] << std::endl;
    // std::cout << points_mem[2].coord[0] << ", " << points_mem[2].coord[1] << std::endl;

    KMEANS<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km;
    km.run(points_mem, hls_clusters_centers);

    // ~~~~~~~ CSR ~~~~~~~
    std::array<COORD_TYPE,N_POINTS> x_data, y_data, crd;
    std::vector<CSR_POS_T> pos;
    // C_TYPE hls_clusters_centers_sd[K_CLSTRS];

    for (int i=0; i<N_POINTS; i++) {
      std::vector<COORD_TYPE> pnt = data_points[i];
      x_data[i] = pnt[0];
      y_data[i] = pnt[1];
    }

    int xx = CSR_conv<COORD_TYPE,MAX_VALUE,N_POINTS>(x_data, y_data, crd, pos);

    std::array<CSR_POS_T, N_POINTS> a_pos;
    
    int id = 0;
    for (auto p : pos) {
      a_pos[id] = p;
      id++;
    }
     
    CSR_CRD_T mem[N_POINTS];
    for (int i=0; i<N_POINTS; i++) {
      mem[i].coord[0] = crd[i];
      mem[i].id_cluster = (CID_TYPE)i;
    }
    
    KMEANS_SD<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_sd;
    km_sd.run(mem, &a_pos[0], xx, hls_clusters_centers_sd);
   
    // std::cout << " " << std::endl;
    std::cout << "BASELINE" << std::setw(14) << "CSR" << std::setw(16) << std::endl;
    for (int i=0; i<K_CLSTRS; i++) {
      std::string tmp;
      tmp = to_string(hls_clusters_centers[i][0].to_int());
      tmp.append(", ");
      tmp.append(to_string(hls_clusters_centers[i][1].to_int()));
      std::cout.width(17); std::cout << std::left << tmp;
      
      tmp = to_string(hls_clusters_centers_sd[i][0].to_int());
      tmp.append(", ");
      tmp.append(to_string(hls_clusters_centers_sd[i][1].to_int()));
      std::cout.width(17); std::cout << std::left << tmp << std::endl;

    }
    
    // int err = 0;
    // for (int i=0; i<N_POINTS; i++) {
    //   if (points_mem1[i].id_cluster != points_mem2[i].id_cluster) {
    //     err++;
    //   }
    // }
    // std::cout << " " << std::endl;
    // std::cout << (float)(N_POINTS - err)/(float)N_POINTS *100.0 << "% correct assignment" << std::endl;
  }
  
  
  CCS_RETURN(0);
}
