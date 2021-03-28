#ifndef K_MEANS_TI_SUMS_H  
#define K_MEANS_TI_SUMS_H 

#include "../datatypes_spec.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int MAX_ITER>
class KMEANS_SUMS {
private:
  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  std::array<DIST_TYPE,K> min_dist_cent;
  std::array<DIST_TYPE,K> cluster_coord_sum;

  // TB
  #ifndef __SYNTHESIS__
    int comp_cnt;
    int mult_cnt;
    int adds_cnt;
    int scal_cnt;
    int macc_cnt;
    int cmac_cnt;
  #endif

  CID_TYPE get_nearest_center(P_TYPE pnt, bool initial, C_TYPE clusters[K]) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 

    #ifndef __SYNTHESIS__
      comp_cnt++;
    #endif

    if(initial) {
      bool first_check = true;

      #ifndef __SYNTHESIS__
        macc_cnt ++;
      #endif

      INIT_FIND_NEAR_CENT: for (int i=0; i<K; i++) {

        #ifndef __SYNTHESIS__
          cmac_cnt ++;
        #endif
          
        DIST_TYPE dist = 0.0;

        // COORD_TYPE pnt_diff;
        INIT_CALC_DIST: for (int j=0; j<DIM; j++) {
          // ac_math::ac_abs(pnt.coord[j] - clusters[i][j], pnt_diff);
          // dist += pnt_diff*pnt_diff;
          dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
        }
        #ifndef __SYNTHESIS__
          adds_cnt += 2*DIM;
          mult_cnt += DIM;
        #endif
        
        #ifndef __SYNTHESIS__
          comp_cnt +=3;
        #endif
        if (dist < min_dist || first_check) {
          min_dist = dist;
          best_cl_id = (CID_TYPE)i;
          first_check = false;
        }
      }

    } else {

      DIST_TYPE sumA = cluster_coord_sum[best_cl_id];
      
      #ifndef __SYNTHESIS__
        macc_cnt ++;
      #endif
    
      min_dist = 0;
      DIST__XA_: for (int j=0; j<DIM; j++) {
        min_dist += (pnt.coord[j] - clusters[best_cl_id][j])*(pnt.coord[j] - clusters[best_cl_id][j]);
      }
      #ifndef __SYNTHESIS__
        adds_cnt += 3*DIM;
        mult_cnt += DIM;
      #endif
      
      #ifndef __SYNTHESIS__
        comp_cnt ++;
        scal_cnt ++;
      #endif
      if (4*min_dist > min_dist_cent[best_cl_id]) {
        FIND_NEAR_CENT: for (int i=0; i<K; i++) {
          
          #ifndef __SYNTHESIS__
           comp_cnt ++;
          #endif
          if ((CID_TYPE)i != best_cl_id) {

            DIST_TYPE sumB = cluster_coord_sum[i];

            DIST_TYPE f = (sumA-sumB)*(sumA-sumB);
            #ifndef __SYNTHESIS__
              adds_cnt ++;
              mult_cnt ++;
            #endif
            
            #ifndef __SYNTHESIS__
              comp_cnt ++;
              scal_cnt ++;
            #endif
            if (4*DIM*min_dist > f) {
              #ifndef __SYNTHESIS__
                cmac_cnt ++;
              #endif
          
              DIST_TYPE dist = 0.0;

              // COORD_TYPE pnt_diff;
              CALC_DIST: for (int j=0; j<DIM; j++) {
                // ac_math::ac_abs(pnt.coord[j] - clusters[i][j], pnt_diff);
                // dist += pnt_diff*pnt_diff;
                dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
              }
              #ifndef __SYNTHESIS__
                adds_cnt += 3*DIM;
                mult_cnt += DIM;
              #endif
              
              #ifndef __SYNTHESIS__
                comp_cnt ++;
              #endif
              if (dist < min_dist) {
                min_dist = dist;
                best_cl_id = (CID_TYPE)i;
                sumA = sumB;
              }
            } 
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
      #ifndef __SYNTHESIS__
        cmac_cnt++;
      #endif
      C_TYPE curr_cent = clusters[i];

      DIST_TYPE coord_sum = 0;
      COORD_SUM: for (int j=0; j<DIM; j++) {
        if (curr_cent[j] > 0)
          coord_sum += curr_cent[j];
        else 
          coord_sum -= curr_cent[j];
      }
      cluster_coord_sum[i] = coord_sum;
      #ifndef __SYNTHESIS__
        comp_cnt++;
        adds_cnt+=DIM;
      #endif

      C2C_DIST_J: for (int j=0; j<K; j++) {
        #ifndef __SYNTHESIS__
          comp_cnt++;
        #endif
        if (i != j) {
          #ifndef __SYNTHESIS__
            cmac_cnt++;
          #endif
          DIST_TYPE dist = 0;
          C2C_DIST_CALC: for (int k=0; k<DIM; k++) {
            dist += (curr_cent[k] - clusters[j][k])*(curr_cent[k] - clusters[j][k]);
          }
          #ifndef __SYNTHESIS__
            adds_cnt +=3*DIM;
            mult_cnt += DIM;
            comp_cnt += 3;
          #endif
          if (min_dist == 0 || dist < min_dist) {
            min_dist = dist;
          }
        }
      }
      min_dist_cent[i] = min_dist;
    }
  };

public:
  KMEANS_SUMS() { 
    #ifndef __SYNTHESIS__ 
      comp_cnt = 0; 
      scal_cnt = 0;
      mult_cnt = 0;
      adds_cnt = 0;
      macc_cnt = 0;
      cmac_cnt = 0;
    #endif
  };
  ~KMEANS_SUMS() {};
    
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
        CID_TYPE new_cl_id = get_nearest_center(curr_pnt, iter==0, clusters_centers);

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
      std::cout << "SumsIneq:" << std::endl;
      std::cout << "    Multiplications -> " << mult_cnt << std::endl;
      std::cout << "    Additions ->       " << adds_cnt << std::endl;
      std::cout << "    Comparisons ->     " << comp_cnt << std::endl;
      std::cout << "    Scalling ->        " << scal_cnt << std::endl;
      std::cout << "    Memory Access ->   " << macc_cnt << std::endl;
      std::cout << "    Clust Mem Acc ->   " << cmac_cnt << std::endl;
      std::cout << "    Iterations ->      " << iter << std::endl;
    #endif

  }; // end run
}; // end class
#endif
