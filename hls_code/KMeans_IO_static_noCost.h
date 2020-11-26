#ifndef K_MEANS_IO_STATIC_NO_COST_H 
#define K_MEANS_IO_STATIC_NO_COST_H

#include "datatypes_spec.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int MAX_ITER>
class KMEANS {
private:
  std::array<C_TYPE,K> clusters;

  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  DIST_TYPE calculate_dist(CID_TYPE cl_id, P_TYPE pnt) {
    DIST_TYPE dist = 0;
    
    CALC_DIST: for (int i=0; i<DIM; i++) {
      dist += (pnt.coord[i] - clusters[cl_id][i])*(pnt.coord[i] - clusters[cl_id][i]);
    }
    
    return dist;
  };

  CID_TYPE get_nearest_center(P_TYPE pnt) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    DIST_TYPE dist = 0.0;
    bool first_check = true;
    FIND_NEAR_CENT: for (int i=0; i<K; i++) {
      dist = calculate_dist((CID_TYPE)i, pnt);

      if (dist < min_dist || first_check) {
        min_dist = dist;
        best_cl_id = (CID_TYPE)i;
        first_check = false;
      }
    }

    return best_cl_id;
  };
   

  void update_cluster_center() {

    UPD_CLUST: for (int i=0; i<K; i++) {
      DATA_DIM_2: for (int j=0; j<DIM; j++) {
        ac_math::ac_div(new_coord[i][j], c_points[i], clusters[i][j]);
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
  KMEANS() {};
  ~KMEANS() {};
    
  #pragma hls_design interface  
  void CCS_BLOCK(run) (P_TYPE points[N], C_TYPE clusters_centers[K]) {

    // DO JOB
    INIT_CLUST: for (int i = 0; i < K; i++) {
      DATA_DIM_1: for (int j=0; j<DIM; j++) {
        clusters[i][j] = points[i].coord[j];
      }
    }

    int iter = 0;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {
      
      keep_going = false;
    
      INIT_CNTR_ACC: for (int i=0; i<K; i++) {
        ac::init_array<AC_VAL_0>(&new_coord[i][0], DIM);
      }
      ac::init_array<AC_VAL_0>(&c_points[0], K);

      ASGN_TO_CLUST: for (int i=0; i<N; i++) {
        P_TYPE curr_pnt = points[i];
        CID_TYPE new_cl_id = get_nearest_center(curr_pnt);

        c_points[new_cl_id]++;
        ACC_NEW_CNTR: for (int j=0; j<DIM; j++) {
          new_coord[new_cl_id][j] += curr_pnt.coord[j];
        }

        if (new_cl_id != curr_pnt.id_cluster) {
          keep_going = true;
          points[i].id_cluster = new_cl_id;
        }
      }

      update_cluster_center();

      iter++;
    }
    

    // WRITE
    WRITE_OUTPUT: for (int i=0; i<K; i++) {
      DATA_DIM_3: for (int j=0; j<DIM; j++) {
        clusters_centers[i][j] = clusters[i][j];
      }
    }
    
  }; // end run
}; // end class
#endif
