#ifndef K_MEANS_IO_STATIC_H 
#define K_MEANS_IO_STATIC_H

#include "datatypes_spec.h"
#include <array>

template<int K, int N, int MAX_ITER>
class KMeans_IO {
private:
  std::array<C_TYPE,K> clusters;

  // std::array<ACCU_TYPE,K> new_x;
  // std::array<ACCU_TYPE,K> new_y;
  // std::array<COUNTER_TYPE,K> c_points;
  ACCU_TYPE new_x[K], new_y[K];
  COUNTER_TYPE c_points[K];

  void initialize_clusters(P_TYPE points[N]) {
    for (int i = 0; i < K; i++) {
      C_TYPE cl;
      cl.id = (CID_TYPE)i;

      // int indx = rand()%(int)N_POINTS;
      int indx = i;

      cl.x = points[indx].x;
      cl.y = points[indx].y;
      
      clusters[i] = cl;
    }
  };

  DIST_TYPE calculate_dist(CID_TYPE cl_id, P_TYPE pnt) {
    DIST_TYPE dist = (pnt.x - clusters[cl_id].x)*(pnt.x - clusters[cl_id].x) + 
                     (pnt.y - clusters[cl_id].y)*(pnt.y - clusters[cl_id].y);
    return dist;
  };

  bool assign_points_to_clusters(P_TYPE points[N], DIST_TYPE &new_cost) {
    DIST_TYPE cost = 0.0;

    bool keep_going = false;
    ac::init_array<AC_VAL_0>(new_x, K);
    ac::init_array<AC_VAL_0>(new_y, K);
    ac::init_array<AC_VAL_0>(c_points, K);
    for (int i=0; i<N; i++) {
      CID_TYPE curr_cl_id = points[i].id_cluster;
      CID_TYPE new_cl_id = get_nearest_center(points[i], cost);

      if (new_cl_id != curr_cl_id) {
        keep_going = true;
        points[i].id_cluster = new_cl_id;
      }
      new_x[new_cl_id] += points[i].x;
      new_y[new_cl_id] += points[i].y;
      c_points[new_cl_id] ++;
    }
    new_cost = cost;
    return keep_going;
  };

  CID_TYPE get_nearest_center(P_TYPE pnt, DIST_TYPE &cost) {
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
    cost += min_dist;
    return best_cl_id;
  };
   

   void update_cluster_center() {

    for (int i=0; i<K; i++) {
      clusters[i].x = new_x[i] / c_points[i];
      clusters[i].y = new_y[i] / c_points[i];
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
    DIST_TYPE old_cost = 0.0;
    DIST_TYPE new_cost = 0.0;
    DIST_TYPE diff = 0.0;

    // DO JOB
    initialize_clusters(points);

    int iter = 0;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {
      keep_going = assign_points_to_clusters(points, new_cost);
      update_cluster_center();

      diff = absolute_diff(old_cost, new_cost);
      if (diff < 0.00001) {
          break;
      }
      old_cost = new_cost;
      iter++;
    }
    

    // WRITE
    for (int i=0; i<K; i++) {
      clusters_centers[i].x = clusters[i].x;
      clusters_centers[i].y = clusters[i].y;
    }
    
  }; // end run
}; // end class
#endif


// DEPR

  // DIST_TYPE calculate_cost(P_TYPE points[N]) {
  //   DIST_TYPE cost = 0.0;
  //   for (int i=0; i<N; i++) {
  //     DIST_TYPE dist = calculate_dist(points[i].id_cluster, points[i]);
  //     cost += dist;
  //   }
  //   return cost;
  // };