// #include "KMeans_nC_SD_CSR.h"
#include "KMeans_nC_TI.h"
#include "KMeans_nC_TI_Hamerly.h" 
#include "KMeans_nC_TI_Elkan.h" 

#include "KMeans_nC_TI_Elkan_HW.h" 
#include "KMeans_nC_TI_our.h"
#include "KMeans_nC.h"
// #include "KMeans.h"

#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>

#include <mc_scverify.h>


template<typename T, int S, int N>
void CSR_conv(std::array<T,N> &x_data, std::array<T,N> &y_data, std::array<T,N> &crd, std::array<CSR_POS_P_TYPE,S> &pos) {
    // INIT
  std::array<int,S> offset;
  ac::init_array<AC_VAL_0>(&offset[0], S);
  
  // FIRST MEMORY READ
  for (int i=0; i<N; i++) {
    offset[y_data[i].to_int()] ++;
  }
  
  // CREATE POS
  pos[0] = 0;
  for (int i=0; i<S-1; i++) {
    pos[i+1] = pos[i]+ offset[i];
  }

  // SECOND MEMORY READ - CREATE CRD
  for (int i=0; i<N; i++) {
    int pos_indx = y_data[i].to_int();
    int indx =  pos[pos_indx] + offset[pos_indx] - 1;
    crd[indx] = x_data[i];
    offset[pos_indx]--;
  }

}


/* 
* HELPER FUNCTIONS
*/

void create_random_data(std::vector<std::pair<COORD_TYPE, COORD_TYPE>> &km_dt_data) {
  for (int i=0; i< N_POINTS; i++) {
    COORD_TYPE x ,y;
    int mulx, muly;

    mulx = rand()%100000;
    // x = 5.1 * mulx;
    x = (COORD_TYPE)mulx;

    muly = rand()%100000;
    // y = 4.3 * muly;
    y = (COORD_TYPE)muly;

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
    C_TYPE hls_clusters_centers_sd[K_CLSTRS];
    C_TYPE hls_clusters_centers_ti[K_CLSTRS];
    C_TYPE hls_clusters_centers_our[K_CLSTRS];
    C_TYPE hls_clusters_centers[K_CLSTRS];
    P_TYPE points_mem1[N_POINTS], points_mem2[N_POINTS], points_mem3[N_POINTS];

    KMEANS<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km;

    // KMEANS_SD<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_sd;

    // KMEANS_TI_H<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_ti;
    // KMEANS_TI_E<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_ti;
    // KMEANS_TI<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_ti;

    KMEANS_E_HW<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_ti;
    KMEANS_OUR<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_our;
    
    std::array<COORD_TYPE,N_POINTS> x_data, y_data, crd;
    std::array<CSR_POS_P_TYPE,POS_SIZE> pos;
    int id = 0;
    for (auto pnt : data_points) {
      points_mem1[id].coord[0] = pnt.first;
      points_mem1[id].coord[1] = pnt.second;

      points_mem2[id].coord[0] = pnt.first;
      points_mem2[id].coord[1] = pnt.second;

      points_mem3[id].coord[0] = pnt.first;
      points_mem3[id].coord[1] = pnt.second;
      
      x_data[id] = pnt.first;
      y_data[id] = pnt.second;

      id++;
    }

    // CSR_conv<COORD_TYPE,POS_SIZE,N_POINTS>(x_data, y_data, crd, pos);

    // CSR_CRD_P_TYPE mem[N_POINTS];
    // for (int i=0; i<N_POINTS; i++) {
    //   mem[i].coord[0] = crd[i];
    //   mem[i].id_cluster = (CID_TYPE)i;
    // }

    // km_sd.run(mem, &pos[0], hls_clusters_centers_sd);
    km.run(points_mem1, hls_clusters_centers);
    km_ti.run(points_mem2, hls_clusters_centers_ti);
    km_our.run(points_mem3, hls_clusters_centers_our);
   
    std::cout << " " << std::endl;
    std::cout << "BASELINE" << std::setw(14) << "ELKAN" << std::setw(16) << "OURS" << std::endl;
    for (int i=0; i<K_CLSTRS; i++) {
      std::string tmp;
      tmp = to_string(hls_clusters_centers[i][0].to_int());
      tmp.append(", ");
      tmp.append(to_string(hls_clusters_centers[i][1].to_int()));
      std::cout.width(17); std::cout << std::left << tmp;
      
      tmp = to_string(hls_clusters_centers_ti[i][0].to_int());
      tmp.append(", ");
      tmp.append(to_string(hls_clusters_centers_ti[i][1].to_int()));
      std::cout.width(17); std::cout << std::left << tmp;
      
      tmp = to_string(hls_clusters_centers_our[i][0].to_int());
      tmp.append(", ");
      tmp.append(to_string(hls_clusters_centers_our[i][1].to_int()));
      std::cout.width(17); std::cout << std::left << tmp << std::endl;

      // std::cout << hls_clusters_centers[i][0] << ", " << hls_clusters_centers[i][1] << std::setw(15-tmp.size());
      // std::cout << hls_clusters_centers_ti[i][0] << ", " << hls_clusters_centers_ti[i][1] << std::setw(15);
      // std::cout << hls_clusters_centers_our[i][0] << ", " << hls_clusters_centers_our[i][1] << std::endl;
    }
    
    int err_ti = 0, err_our = 0;
    for (int i=0; i<N_POINTS; i++) {
      if (points_mem1[i].id_cluster != points_mem2[i].id_cluster) {
        err_ti++;
      }
      if (points_mem1[i].id_cluster != points_mem3[i].id_cluster) {
        err_our++;
      }
    }
    std::cout << " " << std::endl;
    std::cout << "ELKAN: " << (float)(N_POINTS - err_ti)/(float)N_POINTS *100.0 << "% correct assignment" << std::endl;
    std::cout << "OURS:  " << (float)(N_POINTS - err_our)/(float)N_POINTS *100.0 << "% correct assignment" << std::endl;
  }
  
  
  CCS_RETURN(0);
}
