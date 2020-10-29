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
#include "KMeans_datatypes.h"
#include "KMeans_ΙΟ.h"

#include <ac_channel.h>
#include <ac_math.h>

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
}

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
}

int main(int argc, char *argv[]) {
  std::string filename = "";
  std::string method = "";
  
  bool found_file = false;
  bool found_method = false;
  for (int i=0; i<argc; i++) {
    std::cout << argv[i] << " ";

    if (!strcmp(argv[i], "-file")) {
      assert(i+1 < argc);
      filename = (std::string)argv[i+1];
      found_file = true;
    }

    if (!strcmp(argv[i], "-method")) {
      assert(i+1 < argc);
      method = (std::string)argv[i+1];
      std::transform(method.begin(), method.end(), method.begin(), ::toupper);
      found_method = true;
    }
  }
  std::cout << std::endl;
  assert(found_file);
  assert(found_method);

  // READ_DATA FROM FILE
  std::vector<std::pair<COORD_TYPE, COORD_TYPE>> km_dt_data;
  read_file<COORD_TYPE>(filename, km_dt_data);
  std::vector<std::pair<float, float>> kmeans_data;
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
    

  /*
  * Version with defined datatypes
  */ 
  KMeans_datatypes<K_CLSTRS, N_POINTS> km_dt;

  km_dt.set_data_name(filename);
  km_dt.add_data(km_dt_data);
  km_dt.run();
  std::vector<std::pair<COORD_TYPE, COORD_TYPE>> km_dt_centers = km_dt.get_cluster_centers();
  std::vector<CLTID_TYPE> km_dt_p2c = km_dt.get_cluster_of_points();

  /*
  * Version with defined IOs
  * */
  KMeans_IO<K_CLSTRS, N_POINTS, M_ITERAT> km_io;
  
  ac_channel<P_TYPE> km_io_points[N_POINTS];
  ac_channel<CENTER_T> km_io_centers_2_tb[K_CLSTRS];
  ac_channel<CLTID_TYPE> km_io_p2c_2_tb[N_POINTS];
  
  P_TYPE write_points[N_POINTS];
  std::vector<std::pair<COORD_TYPE, COORD_TYPE>> km_io_centers;
  std::vector<CLTID_TYPE> km_io_p2c;
  
  P_TYPE tmp;
  int id=0;
  for (auto pnt : km_dt_data) {
    tmp.x = pnt.first;
    tmp.y = pnt.second;
    tmp.id = id;
    km_io_points[id].write(tmp);
    id++;
  }

  km_io.run(km_io_points, km_io_centers_2_tb, km_io_p2c_2_tb);

  
  CENTER_T tmp_center;
  for (int i=0; i<K_CLSTRS; i++) {
  tmp_center = km_io_centers_2_tb[i].read();
  km_io_centers.push_back(std::make_pair(tmp_center.x, tmp_center.y));
  }

  for (int i=0; i< N_POINTS; i++) {
    km_io_p2c.push_back(km_io_p2c_2_tb[i].read());
  }

  
  /*
  * CHECK RESULTS
  */
  std::vector<std::pair<float, float>> copy_centers_bs;
  std::vector<std::pair<COORD_TYPE, COORD_TYPE>> copy_centers_dt, copy_centers_io;
  std::vector<int> bs_matching;
  std::vector<CLTID_TYPE> dt_matching, io_matching;

  copy_centers_bs = kmeans_centers;
  copy_centers_dt = km_dt_centers;
  copy_centers_io = km_io_centers;
  
  sort(kmeans_centers.begin(), kmeans_centers.end());
  sort(km_dt_centers.begin(), km_dt_centers.end());
  sort(km_io_centers.begin(), km_io_centers.end());

  int it = 0;
  for (auto c : copy_centers_bs) {
    int jt = 0;
    for (auto d : kmeans_centers) {
      if (c == d) {
        bs_matching.push_back(jt);
        break;
      }
      jt++;
    }
    it++;
  }
  
  it = 0;
  for (auto c : copy_centers_dt) {
    int jt = 0;
    for (auto d : km_dt_centers) {
      if (c == d) {
        dt_matching.push_back(jt);
        break;
      }
      jt++;
    }
    it++;
  }

  it = 0;
  for (auto c : copy_centers_io) {
    int jt = 0;
    for (auto d : km_io_centers) {
      if (c == d) {
        io_matching.push_back(jt);
        break;
      }
      jt++;
    }
    it++;
  }
  

  for (int i=0; i<K_CLSTRS; i++) {
    std::cout << "BASELINE: c" << i << "={" << kmeans_centers[i].first << ", " << kmeans_centers[i].second << "}\n";
    std::cout << "DATATYPE: c" << i << "={" << km_dt_centers[i].first  << ", " << km_dt_centers[i].second  << "}\n";
    std::cout << "IO-INTER: c" << i << "={" << km_io_centers[i].first  << ", " << km_io_centers[i].second  << "}\n\n";
  }

  int correct_assign_datatype = 0;
  int correct_assign_io = 0;

  int bs_id;
  CLTID_TYPE dt_id, io_id;

  for (int i=0; i<N_POINTS; i++) {
    bs_id = kmeans_p2c[i];
    dt_id = km_dt_p2c[i];
    io_id = km_io_p2c[i];

    if (bs_matching[bs_id] == dt_matching[dt_id]) {
      correct_assign_datatype++;
    } 
    if (bs_matching[bs_id] == io_matching[io_id]) {
      correct_assign_io++;
    }
  }

  std::cout << "Points placed correctly!" << std::endl;
  std::cout << "DT-version: " << correct_assign_datatype << "/" << N_POINTS << std::endl;
  std::cout << "IO-version: " << correct_assign_io       << "/" << N_POINTS << std::endl;

  return 0;
}