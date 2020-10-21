#ifndef MEAN_SHIFT_H
#define MEAN_SHIFT_H

#include <iostream>
#include <stdlib.h>
#include <vector>
#include <set>
#include <math.h>
#include <algorithm>

#define DEFAULT_RADIUS 4
#define DEFAULT_MAX_ITER 100

struct Cluster
{
  /* data */
  int id;

  float x;
  float y;

  std::set<int> points;
};

struct Point
{
  /* data */
  int id;

  float x;
  float y;
};



class MeanShift {

  enum KERNEL {NONE, GAUSSIAN, EPANECHNIKOV};
  enum DIST_CALC {EUCLIDEAN, MANHATTAN, MAX};

  private:
    std::vector<Cluster> clusters;
    std::vector<Point>   points;

    float radius;
    int maxIterations;
    KERNEL kernel;
    DIST_CALC dist_calculator;

    bool tolerance;

    // only for MATLAB file
    std::string data_name;


    bool check_similarity(std::vector<Cluster> new_clusters);
    void attach_points_to_clusters();

  public:
    
    // Constructor
    MeanShift() {
      radius = DEFAULT_RADIUS;
      maxIterations = DEFAULT_MAX_ITER;
      kernel = NONE;
      dist_calculator = EUCLIDEAN;
      tolerance = 0.0001;
    }

    // Functions to configure clustering
    void set_tolerance(float tol)  {
      tolerance = tol;
    }
    void set_max_iterations(int iterations)  {
      maxIterations = iterations;
    }
    void set_kernel(std::string K) {
      std::transform(K.begin(), K.end(), K.begin(), ::toupper);
      if (K == "NONE") {
        kernel = NONE;
      }else if (K == "GAUSSIAN") {
        kernel = GAUSSIAN;
      }else if (K == "EPANECHNIKOV") {
        kernel = EPANECHNIKOV;
      } else {
        std::cout << "Wrong kernel input. Continuing with DEFAULT kernel (NONE)." << std::endl;
      }
    }
    void set_dist_method(std::string K) {
      std::transform(K.begin(), K.end(), K.begin(), ::toupper);
      if (K == "EUCLIDEAN") {
        dist_calculator = EUCLIDEAN;
      }else if (K == "MANHATTAN") {
        dist_calculator = MANHATTAN;
      }else if (K == "MAX") {
        dist_calculator = MAX;
      }else {
        std::cout << "Wrong input method for distance calculation. Continuing with DEFAULT method (EUCLIDEAN)." << std::endl;
      }
    }
    void set_radius(float rad) {
      radius = rad;
    }

    // Functions to add data
    void add_data(std::vector<std::pair<float, float> > data);
    void add_point(float x, float y);
    

    void initialize_clusters();
    float compute_distance(Point pnt, Cluster cls);
    float compute_distance(Cluster c1, Cluster c2);
    void update_clusters(std::vector<Cluster> new_clusters);

    std::vector<Cluster> compute_weighted_new_clusters();

    float Gaussian_Kernel(float distance, float radius);
    float Epanechnikov_Kernel(float distance, float radius);
    
    // Mean shift run function
    void run();
    

    // Save data file name
    void set_data_name(std::string value) {
      data_name = value;
    }
    // Functions to report results
    void display_clusters();
    void print_clusters();
};


#endif // MEAN_SHIFT_H