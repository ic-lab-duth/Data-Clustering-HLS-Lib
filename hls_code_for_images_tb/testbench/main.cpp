#include "datatypes_spec.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

#include "read_img_header.h"

#include "hls_kmeans.h"
#include <mc_scverify.h>



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
void create_data_from_file(std::string filename, std::vector<std::pair<COORD_TYPE, COORD_TYPE>> &design_data, 
                           std::vector<std::pair<float, float>> &float_data) {
    read_file<COORD_TYPE>(filename, design_data);
    read_file<float>(filename, float_data);
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


void create_random_data(std::vector<std::pair<COORD_TYPE, COORD_TYPE>> &design_data, std::vector<std::pair<float, float>> &float_data) {
  for (int i=0; i< N_POINTS; i++) {
    COORD_TYPE x ,y;
    float fx, fy;
    int mulx = rand()%60000;
    int muly = rand()%50000;
    x = 5.1 * mulx;
    fx = 5.1* mulx;

    y = 4.3 * muly;
    fy = 4.3* muly;
    design_data.push_back(std::make_pair(x,y));
    float_data.push_back(std::make_pair(fx,fy));
  }
};


void my_check_results(P_TYPE data[N_POINTS], CENTER_T centers[K_CLSTRS]) {
  
    std::cout << "\n ~~~~~~~ PRINT  RESULTS ~~~~~~~ \n" << std::endl;
  
  for (int i=0; i<K_CLSTRS; i++) {
    std::cout << "DESIGN CENETRS: c" << i << "={";
    for (int j=0; j<DATA_DIM; j++) {
      std::cout << centers[i].coord[j];
      if (j<DATA_DIM-1) std::cout << ", ";
    } 
    std::cout << "}" << std::endl;
  }
  std::cout << "\n ~~~~~~~ END OF RESULTS ~~~~~~~ " << std::endl;
};







/*
*  MAIN FUNCTION TESTBENCH
*
*/

CCS_MAIN(int argc, char *argv[]) {

  std::string filename = "./../images/test_sunset.bmp";
  std::string img_file = "./../images/results/result.bmp";

  bool testing = false;

  int number_of_tests = 1;

  for (int test = 0; test<number_of_tests; test++) {

    // std::vector<std::pair<COORD_TYPE, COORD_TYPE>> design_data;
    // std::vector<std::pair<float, float>> float_data;

    // // READ_DATA FROM FILE
    // create_data_from_file(filename, design_data, float_data);

    // CREATE RANDOM DATA 
    //create_random_data(design_data, float_data);

    CENTER_T computed_centers[K_CLSTRS];
    P_TYPE data_points[N_POINTS];

    // READ DATA FROM IMAGE
    read_image_data(filename, IMG_HGHT, IMG_WDTH, data_points);

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
    
    if (!testing) {
      /* Version with defined IOs */
      KMeans_IO<K_CLSTRS, N_POINTS, DATA_DIM, M_ITERAT> kmeans;
      
      kmeans.run(data_points, computed_centers);
      
      /* CHECK RESULTS */
      my_check_results(data_points, computed_centers);
      write_image_from_data(img_file, IMG_WDTH, IMG_HGHT, data_points, computed_centers);
    }

  }
    

  CCS_RETURN(0);
}

