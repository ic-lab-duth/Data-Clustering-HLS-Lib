
#include "../../hls_code/K-MEANS/KMeans.h"
#include "../../hls_code/K-MEANS/KMeans_nC.h"
#include "../../hls_code/K-MEANS/KMeans_nC_TI_Elkan_HW.h" 
#include "../../hls_code/K-MEANS/KMeans_nC_TI_Sums.h"

#include <iomanip>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include <mc_scverify.h>


/* 
* HELPER FUNCTIONS
*/

void create_random_data(std::vector<std::vector<COORD_TYPE>> &km_dt_data) {
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
      // x = 5.1 * mulx;
      x = (COORD_TYPE)mulx;

      point.push_back(x);

    }

    km_dt_data.push_back(point);
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
  std::string filename = "dim032.csv";
  std::string method = "";

  bool read_file = true; // true = input data | false = random data
  
  // Select which versions to test
  bool run_baseline = true;   // Baseline K-Means with cost calculation
  bool run_base_noC = true;   // Baseline K-Means without cost calculation
  bool run_elkan_ineq = true; // K-Means without cost, using Elkan's Triangle Inequality
  bool run_elkan_sums = true; // K-Means without cost, using Elkan's TI with Sums
  

  /* 
  *  Configure output printing 
  *  1. Print Center outputs for each version
  *  2. Print the percentage of correct assignment (Compared to baseline)
  */
  bool print_centers = false;
  bool print_p2c_acc = true;  // for this to be true, you must run the baseline. --> run_baseline = true
  

  int number_of_tests = 1;
  for (int test = 0; test<number_of_tests; test++) {

    // CREATE RANDOM DATA 
    std::vector<std::vector<COORD_TYPE>> data_points;
    if (read_file) 
      read_data(data_points, filename);
    else
      create_random_data(data_points);

  
    P_TYPE points_mem[N_POINTS];
    for (int i=0; i<N_POINTS; i++) {
      for (int j=0; j<DATA_DIM; j++) {
        points_mem[i].coord[j] = data_points[i][j];
      }
    }

    
    // ~~~~~~~~~~~ BASELINE K-MEANS ~~~~~~~~~~~
    C_TYPE baseline_clusters[K_CLSTRS];
    CID_TYPE km_baseline_p2c[N_POINTS];
    if (run_baseline) {
      
      for (int i=0; i<N_POINTS; i++) {
        points_mem[i].id_cluster = 0;
      }

      KMEANS<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km;
      km.run(points_mem, baseline_clusters);
      
      for (int i=0; i<N_POINTS; i++) {
        km_baseline_p2c[i] = points_mem[i].id_cluster;
      }
    }

    
    // ~~~~~~~~~~~ BASELINE noCost K-MEANS ~~~~~~~~~~~
    C_TYPE baseline_noC_clusters[K_CLSTRS];
    CID_TYPE km_base_nC_p2c[N_POINTS];
    if (run_base_noC) {      

      for (int i=0; i<N_POINTS; i++) {
        points_mem[i].id_cluster = 0;
      }

      KMEANS_NC<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> kmnc;
      kmnc.run(points_mem, baseline_noC_clusters);

      for (int i=0; i<N_POINTS; i++) {
        km_base_nC_p2c[i] = points_mem[i].id_cluster;
      }
    }

     
    // K-means triangle inequality elkan
    C_TYPE elkan_clusters[K_CLSTRS];
    CID_TYPE km_elkan_p2c[N_POINTS];
    if (run_elkan_ineq) {      
      
      for (int i=0; i<N_POINTS; i++) {
        points_mem[i].id_cluster = 0;
      }

      KMEANS_ELKAN<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_ti;
      km_ti.run(points_mem, elkan_clusters);

      for (int i=0; i<N_POINTS; i++) {
        km_elkan_p2c[i] = points_mem[i].id_cluster;
      }
    }
     
    // K-means triangle inequality Elkan + Sums
    C_TYPE elkan_sums_clusters[K_CLSTRS];
    CID_TYPE km_elkan_sums_p2c[N_POINTS];
    if (run_elkan_sums) {
      
      for (int i=0; i<N_POINTS; i++) {
        points_mem[i].id_cluster = 0;
      }

      KMEANS_SUMS<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> km_sums;
      km_sums.run(points_mem, elkan_sums_clusters);

      for (int i=0; i<N_POINTS; i++) {
        km_elkan_sums_p2c[i] = points_mem[i].id_cluster;
      }
    }
    


    // ~~~~~~~~~~~~~~~~~~~~~~~~ PRINTING RESULTS ~~~~~~~~~~~~~~~~~~~~~~~~~~
    std::cout << " " << std::endl;
    if (print_centers) {
      if (run_baseline)   std::cout << "Baseline" << std::setw(16); 
      if (run_base_noC)   std::cout << "No Cost " << std::setw(16);
      if (run_elkan_ineq) std::cout << "TI Elkan" << std::setw(16);
      if (run_elkan_sums) std::cout << "TI Sums " << std::setw(16);
      std::cout << endl;
    }
    for (int i=0; i<K_CLSTRS; i++) {
      std::string tmp;

      if (run_baseline && print_centers) {
        tmp = to_string(baseline_clusters[i][0].to_int());
        tmp.append(", ");
        tmp.append(to_string(baseline_clusters[i][1].to_int()));
        std::cout.width(16); std::cout << std::left << tmp;
      }
      
      if (run_base_noC && print_centers) {
        tmp = to_string(baseline_noC_clusters[i][0].to_int());
        tmp.append(", ");
        tmp.append(to_string(baseline_noC_clusters[i][1].to_int()));
        std::cout.width(16); std::cout << std::left << tmp;
      }

      if (run_elkan_ineq && print_centers) {
        tmp = to_string(elkan_clusters[i][0].to_int());
        tmp.append(", ");
        tmp.append(to_string(elkan_clusters[i][1].to_int()));
        std::cout.width(16); std::cout << std::left << tmp;
      }
      
      if (run_elkan_sums && print_centers) {
        tmp = to_string(elkan_sums_clusters[i][0].to_int());
        tmp.append(", ");
        tmp.append(to_string(elkan_sums_clusters[i][1].to_int()));
        std::cout.width(16); std::cout << std::left << tmp;
      }
      if (print_centers) std::cout << std::endl;
    }
    
    int err_nc = 0, err_elk = 0, err_sum = 0;
    for (int i=0; i<N_POINTS; i++) {
      if (run_base_noC) {
        if (km_baseline_p2c[i] != km_base_nC_p2c[i]) {
          err_nc++;
        }
      }
      if (run_elkan_ineq) {
        if (km_baseline_p2c[i] != km_elkan_p2c[i]) {
          err_elk++;
        }
      }
      if (run_elkan_sums) {
        if (km_baseline_p2c[i] != km_elkan_sums_p2c[i]) {
          err_sum++;
        }
      }
    }
    if (print_p2c_acc) {
      std::cout << " " << std::endl;
      if (run_base_noC)   std::cout << "Base nC: " << (float)(N_POINTS - err_nc)/(float)N_POINTS *100.0 << "% correct assignment" << std::endl;
      if (run_elkan_ineq) std::cout << "Elkan  : " << (float)(N_POINTS - err_elk)/(float)N_POINTS *100.0 << "% correct assignment" << std::endl;
      if (run_elkan_sums) std::cout << "Sums   : " << (float)(N_POINTS - err_sum)/(float)N_POINTS *100.0 << "% correct assignment" << std::endl;
    }
  }
  
  
  CCS_RETURN(0);
}
