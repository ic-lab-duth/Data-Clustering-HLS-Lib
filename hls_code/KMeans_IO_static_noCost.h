#ifndef K_MEANS_IO_STATIC_H 
#define K_MEANS_IO_STATIC_H

#include "datatypes_spec.h"
#include <array>

template<int K, int N, int DIM, int MAX_ITER>
class KMeans_IO {
private:
  std::array<C_TYPE,K> clusters;

  // ACCU_TYPE new_coord[K][DIM];
  // COUNTER_TYPE c_points[K];
  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  void initialize_clusters(P_TYPE points[N]) {
    for (int i = 0; i < K; i++) {
      C_TYPE cl;
      cl.id = (CID_TYPE)i;

      // int indx = rand()%(int)N_POINTS;
      int indx = i;
      
      for (int j=0; j<DIM; j++) {
        cl.coord[j] = points[indx].coord[j];
      }
      
      clusters[i] = cl;
    }
  };

  DIST_TYPE calculate_dist(CID_TYPE cl_id, P_TYPE pnt) {
    DIST_TYPE dist = 0;
    
    for (int i=0; i<DIM; i++) {
      dist += (pnt.coord[i] - clusters[cl_id].coord[i])*(pnt.coord[i] - clusters[cl_id].coord[i]);
    }
    
    return dist;
  };

  bool assign_points_to_clusters(P_TYPE points[N]) {

    bool keep_going = false;

    ac::init_array<AC_VAL_0>(&new_coord[0][0], K*DIM);
    ac::init_array<AC_VAL_0>(&c_points[0], K);

    for (int i=0; i<N; i++) {
      CID_TYPE curr_cl_id = points[i].id_cluster;
      CID_TYPE new_cl_id = get_nearest_center(points[i]);

      if (new_cl_id != curr_cl_id) {
        keep_going = true;
        points[i].id_cluster = new_cl_id;
      }
      
      c_points[new_cl_id]++;
      for (int j=0; j<DIM; j++) {
        new_coord[new_cl_id][j] += points[i].coord[j];
      }
    }
    return keep_going;
  };

  CID_TYPE get_nearest_center(P_TYPE pnt) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    DIST_TYPE dist = 0.0;
    bool first_check = true;
    for (int i=0; i<K; i++) {
      dist = calculate_dist(clusters[i].id, pnt);

      if (dist < min_dist || first_check) {
        min_dist = dist;
        best_cl_id = clusters[i].id;
        first_check = false;
      }
    }
    return best_cl_id;
  };
   

  void update_cluster_center() {

    for (int i=0; i<K; i++) {
      for (int j=0; j<DIM; j++) {
        clusters[i].coord[j] = new_coord[i][j] / c_points[i];
      }
    }

  };

  DIST_TYPE absolute_diff (DIST_TYPE x, DIST_TYPE y) {
    if (x > y) {
      return x - y;
    } else {
      return y - x;
    }
  };

public:
  KMeans_IO() {};
  ~KMeans_IO() {};
    
  void run(P_TYPE points[N], CENTER_T clusters_centers[K]) {

    // DO JOB
    initialize_clusters(points);

    int iter = 0;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {
      keep_going = assign_points_to_clusters(points);
      update_cluster_center();

      iter++;
    }
    

    // WRITE
    for (int i=0; i<K; i++) {
      for (int j=0; j<DIM; j++) {
        clusters_centers[i].coord[j] = clusters[i].coord[j];
      }
    }
    
  }; // end run
}; // end class
#endif
