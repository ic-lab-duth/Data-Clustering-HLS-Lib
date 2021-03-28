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

#include "../../hls_code/MEAN_SHIFT/MeanShift.h"
#include <mc_scverify.h>

/*
template<int DIM>
void write_pyplot(std::string filename, std::map<PID_TYPE, std::vector<std::pair<COORD_TYPE,COORD_TYPE>>> pmap) {
    std::ofstream out(filename);
    if (out.is_open()) {
      out << "import matplotlib.pyplot as plt\n\n";

      char colors[11] = {'b', 'g', 'r', 'y', 'c', 'k', 'm', 'm', 'c', 'k', 'b'};
      std::string tmp;
      
      for (const auto [key, value] : pmap) {
        out << "x" << key << " = [";
        bool first = true;
        for (const auto v : value) {
            tmp = std::to_string(v.first.to_double());
            if (first) {
                first = false;
                out << v.first;
            } else {
                out << ", " << v.first;
            }
        }
        out << "]\n";

        out << "y" << key << " = [";
        first = true;
        for (const auto v : value) {
            tmp = std::to_string(v.second.to_double());
            if (first) {
                first = false;
                out << v.second;
            } else {
                
                out << ", " << v.second;
            }
        }
        out << "]\n";

        out << "plt.plot(x" << key << ", y" << key << ", '" << colors[key]  << ".')\n";
      }

      out << "plt.savefig(\"data.png\")";

      out.close();

    } else {
      std::cerr << "Unable to open file: " << filename << std::endl;
    }
  };
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



CCS_MAIN(int argc, char *argv[]) {
    std::string filename = "d/data_1.txt";
    std::string method = "MEAN_SHIFT";
   
  
    std::vector<std::vector<COORD_TYPE>> data_points;
    read_data(data_points, "../data_500_16.csv");
    // create_random_data(data_points);
    
    P_TYPE points_mem[N_POINTS];
    C_TYPE clusters_centers[N_POINTS];
    
    for (int i=0; i<N_POINTS; i++) {
      points_mem[i].id_cluster = 0;
      for (int j=0; j<DATA_DIM; j++) {
        points_mem[i].coord[j] = data_points[i][j];
        clusters_centers[i][j] = data_points[i][j];
      }
    }
    

    MEAN_SHIFT<N_POINTS,DATA_DIM,M_ITERAT> ms;
    ms.run(points_mem,clusters_centers, 15);
    
    for (int i=0; i<3; i++) {
        std::cout << clusters_centers[i][0] << ", " << clusters_centers[i][1] << std::endl;
    }

    CCS_RETURN(0);
}
