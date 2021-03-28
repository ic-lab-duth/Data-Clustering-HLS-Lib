#ifndef K_MEANS_ELKAN_H 
#define K_MEANS_ELKAN_H

#include "../datatypes_spec.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int MAX_ITER>
class KMEANS_ELKAN {
private:
  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  std::array<DIST_TYPE,K> min_dist_cent;
  std::array<std::array<DIST_TYPE,K>,K> c2c;


  CID_TYPE get_nearest_center(P_TYPE pnt, C_TYPE clusters[K]) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    
    DIST__XA_: for (int j=0; j<DIM; j++) {
        min_dist += (pnt.coord[j] - clusters[best_cl_id][j])*(pnt.coord[j] - clusters[best_cl_id][j]);
    }

    if (4*min_dist > min_dist_cent[best_cl_id]) { // triangle inequality with 2nd closest

      
      FIND_NEAR_CENT: for (int i=0; i<K; i++) {
                  
          if (4*min_dist > c2c[best_cl_id][i]) {  //triangle inequality with each center-to-center distance

            DIST_TYPE dist = 0.0;
            CALC_DIST: for (int j=0; j<DIM; j++) {
              dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
            }
            
            if (dist < min_dist) {
              min_dist = dist;
              best_cl_id = (CID_TYPE)i;
            }
          } 
        
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

  void cluster_to_cluster_distance(C_TYPE clusters[K]) {
    C2C_DIST_I: for (int i=0; i<K; i++) {
      DIST_TYPE min_dist=0;
      C_TYPE curr_cent = clusters[i];
      C2C_DIST_J: for (int j=0; j<K; j++) {

        if (i != j) {

          DIST_TYPE dist = 0;
          C2C_DIST_CALC: for (int k=0; k<DIM; k++) {
            dist += (curr_cent[k] - clusters[j][k])*(curr_cent[k] - clusters[j][k]);
          }

          if (min_dist == 0 || dist < min_dist) {
            min_dist = dist;
          }
          c2c[i][j] = dist;
        }
      }
      min_dist_cent[i] = min_dist;
    }
  };

public:
  KMEANS_ELKAN() {};
  ~KMEANS_ELKAN() {};
    
  #pragma hls_design interface  
  void CCS_BLOCK(run) (P_TYPE points[N], C_TYPE clusters_centers[K]) {

    // DO JOB
    INIT_CLUST: for (int i = 0; i < K; i++) {
      IC_DATA_DIM: for (int j=0; j<DIM; j++) {
        clusters_centers[i][j] = points[i].coord[j];
      }
    }

    int iter = 0;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {

      // CALCULATE CENTER TO CENTER DISTANCES
      cluster_to_cluster_distance(clusters_centers);
      
      keep_going = false;
    
      INIT_CNTR_ACC: for (int i=0; i<K; i++) {
        ac::init_array<AC_VAL_0>(&new_coord[i][0], DIM);
      }
      ac::init_array<AC_VAL_0>(&c_points[0], K);

      ASGN_TO_CLUST: for (int i=0; i<N; i++) {
        P_TYPE curr_pnt = points[i];
        CID_TYPE new_cl_id = get_nearest_center(curr_pnt, clusters_centers);
        
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

  }; // end run
}; // end class
#endif
