#include "sparse_generation.h"

#include <iostream>


typedef int T1;
typedef float T2;

int main (int argc, char **argv) {

  static int N = 500;
  static int K = 16;
  static int DIM = 2;
  static float MAXVAL = 255.0;
  static float SPREAD = 140.0;
  SPARSE_GEN<T1, T2> generator(N, K, DIM, MAXVAL, SPREAD);
  generator.generate();

  std::vector<std::vector<T1>> centroids = generator.getCentroids();
  std::vector<std::vector<std::vector<T1>>> clusters = generator.getClusters();

  for (int i=0; i<K; i++) {
      std::cout << centroids.at(i)[0] << ", " << centroids.at(i)[1] << std::endl;
  }

  std::string dfilename, cfilename;
  dfilename = "gen_datasets/data";
  dfilename.append("_");
  dfilename.append(std::to_string(N));
  dfilename.append("_");
  dfilename.append(std::to_string(K));

  cfilename = "gen_datasets/centroids";
  cfilename.append("_");
  cfilename.append(std::to_string(N));
  cfilename.append("_");
  cfilename.append(std::to_string(K));
  cfilename.append(".csv");

  generator.write_pyplot(dfilename);
  generator.write_data(dfilename + ".csv");
  generator.write_centroids(cfilename);

  system("python3 plot.py");
  system("rm plot.py");

  return 0;
}