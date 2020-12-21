#ifndef K_MEANS_IO_STATIC_NO_COST_EW_H 
#define K_MEANS_IO_STATIC_NO_COST_EW_H

#include "datatypes_spec.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int MAX_ITER>
class KMEANS_E_HW {
private:
  std::array<C_TYPE,K> clusters;

  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  std::array<DIST_TYPE,K> min_dist_cent;

  // TB
  int cnt_calc;

  CID_TYPE get_nearest_center(P_TYPE pnt, bool initial) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    
    if(initial) {

      bool first_check = true;
      FIND_NEAR_CENT: for (int i=0; i<K; i++) {
          
        DIST_TYPE dist = 0.0;

        // COORD_TYPE pnt_diff;
        CALC_DIST: for (int j=0; j<DIM; j++) {
          // ac_math::ac_abs(pnt.coord[j] - clusters[i][j], pnt_diff);
          // dist += pnt_diff*pnt_diff;
          dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
        }
        cnt_calc++;
        
        if (dist < min_dist || first_check) {
          min_dist = dist;
          best_cl_id = (CID_TYPE)i;
          first_check = false;
        }
      }

    } else {
      min_dist = 0;
      for (int j=0; j<DIM; j++) {
        min_dist += (pnt.coord[j] - clusters[pnt.id_cluster][j])*(pnt.coord[j] - clusters[pnt.id_cluster][j]);
      }
      cnt_calc++;
      
      DIST_TYPE tmp1, tmp2;
      ac_math::ac_sqrt(min_dist, tmp1);
      ac_math::ac_sqrt(min_dist_cent[pnt.id_cluster], tmp2);
      // ac_math::ac_div(min_dist_cent[pnt.id_cluster], (DIST_TYPE)4, tmp);
      // tmp = min_dist_cent[pnt.id_cluster];
      if (2*tmp1 > tmp2) {

        for (int i=0; i<K; i++) {
          
          if ((CID_TYPE)i != pnt.id_cluster) {
          
            DIST_TYPE dist = 0.0;

            // COORD_TYPE pnt_diff;
            for (int j=0; j<DIM; j++) {
              // ac_math::ac_abs(pnt.coord[j] - clusters[i][j], pnt_diff);
              // dist += pnt_diff*pnt_diff;
              dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
            }
            cnt_calc++;
            
            if (dist < min_dist) {
              min_dist = dist;
              best_cl_id = (CID_TYPE)i;
            }
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
  KMEANS_E_HW() { cnt_calc = 0; };
  ~KMEANS_E_HW() {};
    
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

      // CALCULATE CENTER TO CENTER DISTANCES
      for (int i=0; i<K; i++) {
        DIST_TYPE min_dist=0;
        for (int j=0; j<K; j++) {
          if (i != j) {
            DIST_TYPE dist = 0;
            for (int k=0; k<DIM; k++) {
              dist += (clusters[i][k] - clusters[j][k])*(clusters[i][k] - clusters[j][k]);
            }
            #ifndef __SYNTHESIS__
              cnt_calc++;
            #endif
            if (min_dist == 0 || dist < min_dist) {
              min_dist = dist;
            }
          }
        }
        min_dist_cent[i] = min_dist;
      }
      
      keep_going = false;
    
      INIT_CNTR_ACC: for (int i=0; i<K; i++) {
        ac::init_array<AC_VAL_0>(&new_coord[i][0], DIM);
      }
      ac::init_array<AC_VAL_0>(&c_points[0], K);

      ASGN_TO_CLUST: for (int i=0; i<N; i++) {
        P_TYPE curr_pnt = points[i];
        CID_TYPE new_cl_id = get_nearest_center(curr_pnt, iter==0);

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
    std::cout << "Elkan_HW: Caclulation -> " << cnt_calc << std::endl;
    std::cout << " \t  Iterations -> " << iter << std::endl;

  }; // end run
}; // end class
#endif
