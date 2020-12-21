#ifndef K_MEANS_IO_STATIC_H 
#define K_MEANS_IO_STATIC_H

#include "datatypes_spec.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int MAX_ITER>
class KMEANS {
private:
  std::array<C_TYPE,K> clusters;

  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  CID_TYPE get_nearest_center(P_TYPE pnt, DIST_TYPE &cost) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    bool first_check = true;
    FIND_NEAR_CENT: for (CID_TYPE i=0; i<K; i++) {
    
      DIST_TYPE dist = 0.0;
      
      // COORD_TYPE pnt_diff;
      CALC_DIST: for (int j=0; j<DIM; j++) {
        // ac_math::ac_abs(pnt.coord[j] - clusters[i][j], pnt_diff);
        // dist += pnt_diff*pnt_diff;
        dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
      }

      if (dist < min_dist || first_check) {
        min_dist = dist;
        best_cl_id = (CID_TYPE)i;
        first_check = false;
      }
    }
    cost += min_dist;
    return best_cl_id;
  };
   

  void update_cluster_center() {

    UPD_CLUST: for (int i=0; i<K; i++) {
      if (c_points[i] > 0) {
        DATA_DIM_2: for (int j=0; j<DIM; j++) {
          ac_math::ac_div(new_coord[i][j], c_points[i], clusters[i][j]);
        }
      }
    }

  };


public:
  KMEANS() {};
  ~KMEANS() {};
  
  #pragma hls_design interface  
  void CCS_BLOCK(run) (P_TYPE points[N], C_TYPE clusters_centers[K]) {
    DIST_TYPE old_cost = 0.0;
    DIST_TYPE new_cost = 0.0;
    DIST_TYPE diff = 0.0;

    // DO JOB
    INIT_CLUST: for (int i = 0; i < K; i++) {
      DATA_DIM_1: for (int j=0; j<DIM; j++) {
        clusters[i][j] = points[i].coord[j];
      }
    }

    int iter = 0;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {
      
      DIST_TYPE cost = 0.0;

      keep_going = false;
      
      INIT_CNTR_ACC: for (int i=0; i<K; i++) {
        ac::init_array<AC_VAL_0>(&new_coord[i][0], DIM);
      }
      ac::init_array<AC_VAL_0>(&c_points[0], K);

      ASGN_TO_CLUST: for (int i=0; i<N; i++) {
        P_TYPE curr_pnt = points[i];
        CID_TYPE new_cl_id = get_nearest_center(curr_pnt, cost);

        c_points[new_cl_id]++;
        ACC_NEW_CNTR: for (int j=0; j<DIM; j++) {
          new_coord[new_cl_id][j] += curr_pnt.coord[j];
        }

        if (new_cl_id != curr_pnt.id_cluster) {
          keep_going = true;
          points[i].id_cluster = new_cl_id;
        }
      }
      new_cost = cost;


      update_cluster_center();

      ac_math::ac_abs(old_cost - new_cost, diff);
      if (diff < 0.00001) {
          break;
      }
      old_cost = new_cost;
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
