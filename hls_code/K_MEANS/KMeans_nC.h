#ifndef K_MEANS_NO_COST_H 
#define K_MEANS_NO_COST_H

#include "../datatypes_spec.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int MAX_ITER>
class KMEANS_NC {
private:
  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  // TB
  #ifndef __SYNTHESIS__
  int comp_cnt;
  int mult_cnt;
  int adds_cnt;
  int scal_cnt;
  int macc_cnt;
  #endif

  CID_TYPE get_nearest_center(P_TYPE pnt, C_TYPE clusters[K]) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    bool first_check = true;
    FIND_NEAR_CENT: for (int i=0; i<K; i++) {
      C_TYPE curr_cluster = clusters[i];
      DIST_TYPE dist = 0.0;

      // COORD_TYPE pnt_diff;
      CALC_DIST: for (int j=0; j<DIM; j++) {
        // ac_math::ac_abs(pnt.coord[j] - clusters[i][j], pnt_diff);
        // dist += pnt_diff*pnt_diff;
        dist += (pnt.coord[j] - curr_cluster[j])*(pnt.coord[j] - curr_cluster[j]);
      }
      #ifndef __SYNTHESIS__
      adds_cnt += 3*DIM;
      mult_cnt += DIM;
      #endif
      
      #ifndef __SYNTHESIS__
      comp_cnt += 3;
      #endif
      if (dist < min_dist || first_check) {
        min_dist = dist;
        best_cl_id = (CID_TYPE)i;
        first_check = false;
      }
    }

    return best_cl_id;
  };
   

  void update_cluster_center(C_TYPE clusters[K]) {

    UPD_CLUST: for (int i=0; i<K; i++) {
      if (c_points[i] > 0) {
        UC_DATA_DIM: for (int j=0; j<DIM; j++) {
          ac_math::ac_div(new_coord[i][j], c_points[i], clusters[i][j]);
        }
      }
    }

  };

public:
  KMEANS_NC() { 
    #ifndef __SYNTHESIS__ 
    comp_cnt = 0; 
    scal_cnt = 0;
    mult_cnt = 0;
    adds_cnt = 0;
    macc_cnt = 0;
    #endif
  };
  ~KMEANS_NC() {};
    
  #pragma hls_design interface  
  void CCS_BLOCK(run) (P_TYPE points[N], C_TYPE clusters_centers[K]) {
    

    INIT_CLUST: for (int i = 0; i < K; i++) {
      IC_DATA_DIM: for (int j=0; j<DIM; j++) {
        clusters_centers[i][j] = points[i].coord[j];
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
        CID_TYPE new_cl_id = get_nearest_center(curr_pnt, clusters_centers);
        #ifndef __SYNTHESIS__
        macc_cnt ++;
        #endif

        c_points[new_cl_id]++;
        ACC_NEW_CNTR: for (int j=0; j<DIM; j++) {
          new_coord[new_cl_id][j] += curr_pnt.coord[j];
        }

        if (new_cl_id != curr_pnt.id_cluster) {
          keep_going = true;
          points[i].id_cluster = new_cl_id;
        }
      }

      update_cluster_center(clusters_centers);

      iter++;
    }
    #ifndef __SYNTHESIS__
      std::cout << "Baseline NoCost:" << std::endl;
      std::cout << "    Multiplications -> " << mult_cnt << std::endl;
      std::cout << "    Additions ->       " << adds_cnt << std::endl;
      std::cout << "    Comparisons ->     " << comp_cnt << std::endl;
      std::cout << "    Scalling ->        " << scal_cnt << std::endl;
      std::cout << "    Memory Access ->   " << macc_cnt << std::endl;
      std::cout << "    Iterations ->      " << iter << std::endl;
    #endif

  }; // end run
}; // end class
#endif
