#ifndef SPARSE_GEN_H
#define SPARSE_GEN_H

#include <vector>
#include <limits>
#include <math.h>
#include <fstream>
#include <string>
#include <iostream>


template<typename T1, typename T2>
class SPARSE_GEN {
private:
  
  T1 maxVal;
  T2 spread;
  int N, K, DIM;

  std::vector<std::vector<T1>> centroids;
  std::vector<std::vector<std::vector<T1>>> clusters;

  // returns a normalized m x n Matrix
  std::vector<std::vector<float>> rand_normalized(int m=1, int n=1) {
    std::vector<std::vector<float>> A;
    for (int i=0; i<m; i++) {
      std::vector<float> Aij;
      for (int j=0; j<n; j++) {
        float x = (fmodf(rand(), maxVal) + 1) / maxVal;
        Aij.push_back(x);
      }
      A.push_back(Aij);
    }
    return A;
  };

  // returns a random n x m Matrix, where 0 < A(i,j) < 1
  std::vector<std::vector<float>> rand_float(int m=1, int n=1) {
    std::vector<std::vector<float>> A;
    for (int i=0; i<m; i++) {
      std::vector<float> row;
      for (int j=0; j<n; j++) {
        float x = static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
        row.push_back(x);
      }
      A.push_back(row);
    }
    return A;
  };

  // returns randomized Centroids coordinates
  std::vector<T1> randV() {
    std::vector<std::vector<float>> tmp = rand_float(1,DIM);
    std::vector<T1> V;

    for (auto x : tmp.at(0)) {
      V.push_back(x*maxVal+spread);
    }

    return V;
  };
  
  // calculates distance between centroids
  float distance(std::vector<T1> c1, std::vector<T1> c2) {
    float dist = 0.0;
    for (int i=0; i<DIM; i++) {
      dist += pow(c1[i]-c2[i], 2);
    }
    return sqrt(dist);
  };

  // Creates a new Centroid
  std::vector<T1> newCentroid() {
    float min = std::numeric_limits<T1>::max();
    std::vector<T1> newC;

    while (min > 2*pow(spread,2)) {
      newC = randV();
      for (auto c : centroids) {
        float dist = distance(newC, c);
        if (dist < min) {
          min = dist;
        }
      }
    }
    return newC;   
  };


public:
  SPARSE_GEN(int n, int k, int dim, T1 mv, T2 spr) {
    N = n;
    K = k;
    DIM = dim;
    maxVal = mv;
    spread = spr;
  };
  ~SPARSE_GEN() {};

  void generate() {
    for (int i=0; i<K; i++){
      if (centroids.size() == 0) {
        centroids.push_back(randV());
      } else {
        centroids.push_back(newCentroid());
      }
      float r = rand_float()[0][0];
      while (r < 0.7) {
        r = rand_float()[0][0];
      }
      int NK = floor((r+1)*N/(K*2));
      T2 variance = spread;

      std::vector<std::vector<T1>> cluster_K;
      for (int j=0; j<DIM; j++) {
        
        variance = rand_normalized()[0][0] * variance;
        float corr = rand_float()[0][0] - 0.5;
        std::vector<std::vector<float>> coord = rand_normalized(NK, 1);

        std::vector<T1> points_on_J;
        for (int k=0; k<NK; k++) {
          T1 current = coord[k][0] * variance;
          if (j>1) {
            current = cluster_K[k][j-1]*corr + current*sqrt(1-pow(corr,2));    
          }
          float t = rand_float()[0][0];
          while (t < 0.4 && t > 0.6) {
            t = rand_float()[0][0];
          }
          current = current*(rand_float()[0][0]-0.5) + centroids[i][j]; // my additdion ---->>>> *(rand_float()[0][0]-0.5)
          points_on_J.push_back(current);
        }
        cluster_K.push_back(points_on_J);
      }
      clusters.push_back(cluster_K);
    }
  }

  std::vector<std::vector<T1>> getCentroids() {
    return centroids;
  };

  std::vector<std::vector<std::vector<T1>>> getClusters() {
    return clusters;
  };

  
  void write_pyplot(std::string filename) {
    std::ofstream out("plot.py");
    if (out.is_open()) {
      out << "import matplotlib.pyplot as plt\n\n";

      for (int j=0; j<DIM; j++) {
        out << "d" << j << " = [";
        for (int k=0; k<K; k++) {
          for (int i=0; i<clusters[k][j].size(); i++) {
            out << clusters[k][j][i];
            if (k < K-1) out << ", ";
            else if (i < clusters[k][j].size()-1) out << ", ";
          }
        }
        out << "]\n";
      }

      out << "plt.plot(";
      for (int j=0; j<DIM; j++) {
        out << "d" << j;
        if (j<DIM-1) out << ", ";
      }
      out << ", \".\")\n";
      // out << "plt.savefig(\"data.png\")";
      out << "plt.savefig(\"" << filename << ".png\")";

      out.close();

    } else {
      std::cerr << "Unable to open file: plot.py" << std::endl;
    }
  };


  void write_data(std::string filename) {
    std::ofstream out(filename);
    if (out.is_open()) {


      for (int k=0; k<K; k++) {
        for (int i=0; i<clusters[k][0].size(); i++) {
          for (int j=0; j<DIM; j++) {
            out << clusters[k][j][i];
            if (j < DIM-1) out << ";";
          }
          out << "\n";
        }
      }

      out.close();

    } else {
      std::cerr << "Unable to open file: " << filename << std::endl;
    }
  };

  void write_centroids(std::string filename) {
    std::ofstream out(filename);
    if (out.is_open()) {


      for (int k=0; k<K; k++) {
        for (int j=0; j<DIM; j++) {
          out << centroids[k][j];
          if (j < DIM-1) out << ";";
        }
        out << "\n";
      }

      out.close();

    } else {
      std::cerr << "Unable to open file: " << filename << std::endl;
    }
  };

};
#endif
