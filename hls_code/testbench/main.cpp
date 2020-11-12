#include "datatypes_spec.h"

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

#include "KMeans_basic.h"
#include "KMeans_ΙΟ_static.h"

#include <ac_channel.h>
#include <ac_math.h>


// DON'T TOUCH
bool read_line_as_tokens(std::istream &is, std::vector<std::string> &tokens){
  tokens.clear();

  std::string line;
  getline(is, line);
  while (is && tokens.empty()){
    std::string token = "";
    for (unsigned i = 0; i < line.size(); ++i){
        char currChar = line[i];
      if (currChar==' '){
        if (!token.empty()){
            // Add the current token to the list of tokens
            tokens.push_back(token);
            token.clear();
        }
        // else if the token is empty, simply skip the whitespace or special char
      }else{
        // Add the char to the current token
        token.push_back(currChar);
      }
    }

    if (!token.empty()){
      tokens.push_back(token);
    }

    if (tokens.empty())
      // Previous line read was empty. Read the next one.
      std::getline(is, line);
  }

  return !tokens.empty();
};

// DON'T TOUCH
template<typename T>
bool read_file(std::string file, std::vector<std::pair<T, T> >& data) {
  std::ifstream dot_ops((file).c_str());
  if(!dot_ops.good()){
      std::cout << "cannot read file : " << file << std::endl;
      return false;
  }

  data.clear();

  std::vector<std::string> tokens;
  bool valid = read_line_as_tokens(dot_ops, tokens);
  int line=1;
  
  bool found = false;
  std::string pin_name;
  while (valid){
    if (tokens.size() != 0) {
      if (tokens.size() != 2) {
        std::cout << "Error in line " << line << ": Tokens are ";
        for (auto token : tokens) {
            std::cout << "'" << token << "' ";
        }
        std::cout << std::endl;
        assert(false);
      }
      
      data.push_back(std::make_pair(std::stof(tokens[0]), std::stof(tokens[1])));
    }

    valid = read_line_as_tokens(dot_ops, tokens);
    line++;
  }
  dot_ops.close();
  return found;
};
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

/* 
* HELPER FUNCTIONS
*/

// DON'T TOUCH
void create_data_from_file(std::string filename, std::vector<std::pair<COORD_TYPE, COORD_TYPE>> &km_dt_data, 
                           std::vector<std::pair<float, float>> &kmeans_data) {
    read_file<COORD_TYPE>(filename, km_dt_data);
    read_file<float>(filename, kmeans_data);
    size_t pos = 0;
    std::string token;
    while ((pos = filename.find("/")) != std::string::npos) {
      token = filename.substr(0, pos);
      filename.erase(0, pos + 1);
    }
    if ((pos = filename.find(".")) != std::string::npos) {
      filename = filename.substr(0, pos);
    }
};

//DON'T TOUCH
void extract_data(P_TYPE km_io_points[N_POINTS], CENTER_T km_io_centers_2_tb[K_CLSTRS], 
                  std::vector<std::pair<float, float>> &km_io_centers, std::vector<int> &km_io_p2c) {
  CENTER_T tmp_center;
  for (int i=0; i<K_CLSTRS; i++) {
  tmp_center = km_io_centers_2_tb[i];
  km_io_centers.push_back(std::make_pair(tmp_center.x.to_double(), tmp_center.y.to_double()));
  }

  for (int i=0; i< N_POINTS; i++) {
    km_io_p2c.push_back(km_io_points[i].id_cluster.to_int());
  }
}


void create_random_data(std::vector<std::pair<COORD_TYPE, COORD_TYPE>> &km_dt_data, std::vector<std::pair<float, float>> &kmeans_data) {
  for (int i=0; i< N_POINTS; i++) {
    COORD_TYPE x ,y;
    float fx, fy;
    int mulx = rand()%60000;
    int muly = rand()%50000;
    x = 5.1 * mulx;
    fx = 5.1* mulx;

    y = 4.3 * muly;
    fy = 4.3* muly;
    km_dt_data.push_back(std::make_pair(x,y));
    kmeans_data.push_back(std::make_pair(fx,fy));
  }
};

void my_check_results(std::vector<std::pair<float, float>> &km_io_centers, 
                    std::vector<int> &km_io_p2c, 
                    std::vector<std::pair<float, float>> &kmeans_data,
                    std::vector<std::pair<float, float>> &kmeans_centers,
                    std::vector<int> &kmeans_p2c) {
                 
  std::vector<std::pair<float, float>> copy_centers_bs;
  std::vector<std::pair<float, float>> copy_centers_io;
  std::vector<int> bs_matching;
  std::vector<int> io_matching;
  
  // Calculate centers based on design output
  std::vector<int> tb_points_per_center, baseline_cluster_size;
  std::vector<std::pair<float, float>> tb_centers_check;
  for (int i=0; i<K_CLSTRS; i++) {
    tb_centers_check.push_back(std::make_pair(0.0, 0.0));
    tb_points_per_center.push_back(0);
    baseline_cluster_size.push_back(0);
  }
  for (int i=0; i<N_POINTS; i++) {
    tb_centers_check[km_io_p2c[i]].first += kmeans_data[i].first;
    tb_centers_check[km_io_p2c[i]].second += kmeans_data[i].second;
    tb_points_per_center[km_io_p2c[i]]++;
    baseline_cluster_size[kmeans_p2c[i]]++;
  }
  for (int i=0; i<K_CLSTRS; i++) {
    tb_centers_check[i].first = tb_centers_check[i].first/tb_points_per_center[i];
    tb_centers_check[i].second = tb_centers_check[i].second/tb_points_per_center[i];
  }
  sort(tb_centers_check.begin(), tb_centers_check.end());


  copy_centers_bs = kmeans_centers;
  sort(kmeans_centers.begin(), kmeans_centers.end());

  copy_centers_io = km_io_centers;
  sort(km_io_centers.begin(), km_io_centers.end());

  int it = 0;
  for (auto c : kmeans_centers) {
    int jt = 0;
    for (auto d : copy_centers_bs) {
      if (c == d) {
        bs_matching.push_back(jt);
        break;
      }
      jt++;
    }
    it++;
  }
  
  it = 0;
  for (auto c : km_io_centers) {
    int jt = 0;
    for (auto d : copy_centers_io) {
      if (c == d) {
        io_matching.push_back(jt);
        break;
      }
      jt++;
    }
    it++;
  }
  

  
  int correct_assign_datatype = 0;
  int correct_assign_io = 0;
  int bs_id, io_id;
  for (int i=0; i<N_POINTS; i++) {
    for (int j=0; j<K_CLSTRS; j++) {
      if (kmeans_p2c[i] == bs_matching[j]) {
        bs_id = j;
      }
    }
    
    for (int j=0; j<K_CLSTRS; j++) {
      if (km_io_p2c[i] == io_matching[j]) {
        io_id = j;
      }
    }
    if (io_id == bs_id) {
      correct_assign_io++;
    }
    // else {
    //   std::cout << "In io version, point " << i << " = {" << write_points[i].x << ", " << write_points[i].y << "}";
    //   std::cout << " assigned to cluster " << io_matching[io_id] << " instead of " << bs_matching[bs_id] << std::endl;
    // }
  }

  std::cout << "Points placed correctly!" << std::endl;
  std::cout << "IO-version: " << correct_assign_io       << "/" << N_POINTS << std::endl;


  for (int i=0; i<K_CLSTRS; i++) {
    std::cout << "\nBASELINE: c" << i << "={" << kmeans_centers[i].first << ", " << kmeans_centers[i].second << "} -> " << baseline_cluster_size[bs_matching[i]] << "\n";
    std::cout << "IO-INTER: c" << i << "={" << km_io_centers[i].first  << ", " << km_io_centers[i].second << "} -> " << tb_points_per_center[io_matching[i]]  << "\n";
    std::cout << "TB_CNTRS: c" << i << "={" << tb_centers_check[i].first  << ", " << tb_centers_check[i].second  << "}\n";
  }
};







/*
*  MAIN FUNCTION TESTBENCH
*
*/

int main(int argc, char *argv[]) {
  std::string filename = "./../clustering_data/data_1.txt";
  std::string method = "";
  

  int number_of_tests = 1;

  for (int test = 0; test<number_of_tests; test++) {

    std::vector<std::pair<COORD_TYPE, COORD_TYPE>> km_dt_data;
    std::vector<std::pair<float, float>> kmeans_data;

    // // READ_DATA FROM FILE
    // create_data_from_file(filename, km_dt_data, kmeans_data);

    // CREATE RANDOM DATA 
    create_random_data(km_dt_data, kmeans_data);
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
    

    /*
    * Baseline version ( implemented by dimitris )
    */
    int k = K_CLSTRS;
    int iter = M_ITERAT;
    KMeans_basic kmeans(iter, k);

    kmeans.set_data_name(filename);
    kmeans.add_data(kmeans_data);
    kmeans.initialize_clusters();
    kmeans.run();
    std::vector<std::pair<float, float>> kmeans_centers = kmeans.get_cluster_centers();
    std::vector<int> kmeans_p2c = kmeans.get_cluster_of_points();
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

    /*
    * Version with defined IOs
    */
    KMeans_IO<K_CLSTRS, N_POINTS, M_ITERAT> km_io;
    
    CENTER_T km_io_centers_2_tb[K_CLSTRS];
    P_TYPE km_io_points[N_POINTS];

    int id = 0;
    for (auto pnt : km_dt_data) {
      km_io_points[id].x = pnt.first;
      km_io_points[id].y = pnt.second;
      km_io_points[id].id = id;
      id++;
    }

    km_io.run(km_io_points, km_io_centers_2_tb);

    std::vector<std::pair<float, float>> km_io_centers;
    std::vector<int> km_io_p2c;
    extract_data(km_io_points, km_io_centers_2_tb, km_io_centers, km_io_p2c);
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
    
    /*
    * CHECK RESULTS
    */
    my_check_results(km_io_centers, km_io_p2c, kmeans_data, kmeans_centers, kmeans_p2c);

  }
    

  return 0;
}

