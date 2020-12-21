#ifndef K_MEANS_IO_STATIC_NO_COST_OUR_H 
#define K_MEANS_IO_STATIC_NO_COST_OUR_H

#include "datatypes_spec.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int MAX_ITER>
class KMEANS_OUR {
private:
  std::array<C_TYPE,K> clusters;

  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  // TB
  int cnt_calc;

  CID_TYPE get_nearest_center(P_TYPE pnt, bool initial) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    if (initial) {
      best_cl_id = 0;
    }

    DIST_TYPE sum1 = 0, sum2 = 0;
    
    for (int j=0; j<DIM; j++) {
      min_dist += (pnt.coord[j] - clusters[best_cl_id][j])*(pnt.coord[j] - clusters[best_cl_id][j]);
      sum1 += clusters[best_cl_id][j];
    }
    cnt_calc++;


    FIND_NEAR_CENT: for (int i=0; i<K; i++) {
      
      if (i != pnt.id_cluster) {

        sum2 = 0;
        for (int j=0; j<DIM; j++) {
          sum2 += clusters[i][j];
        } 


        if ( (sum1-sum2).to_double()*(sum1-sum2).to_double() < 8*min_dist.to_double()) {
      
          DIST_TYPE dist = 0.0;

          CALC_DIST: for (int j=0; j<DIM; j++) {
            dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
          }
          cnt_calc++;
          
          if (dist < min_dist) {
            min_dist = dist;
            best_cl_id = (CID_TYPE)i;
            sum1 = sum2;
          }
        }
      }
    }

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
  KMEANS_OUR() { cnt_calc = 0; };
  ~KMEANS_OUR() {};
    
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
        CID_TYPE new_cl_id = get_nearest_center(curr_pnt, iter == 0);

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
    std::cout << "OursIneq: Caclulation -> " << cnt_calc << std::endl;
    std::cout << " \t  Iterations -> " << iter << std::endl;

  }; // end run
}; // end class
#endif
