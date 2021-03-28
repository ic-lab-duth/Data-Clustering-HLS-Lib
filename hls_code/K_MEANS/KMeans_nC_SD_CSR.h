#ifndef K_MEANS_IO_STATIC_NO_COST_SD_CSR_H 
#define K_MEANS_IO_STATIC_NO_COST_SD_CSR_H

#include "../datatypes_extra_CSR.h"
#include <mc_scverify.h>

#pragma hls_design
template<int K, int N, int DIM, int POS_SIZE, int MAX_ITER>
class KMEANS_SD {
private:
  std::array<std::array<ACCU_TYPE, DIM>, K> new_coord;
  std::array<COUNTER_TYPE, K> c_points;

  std::array<std::array<DIST_TYPE,K>,K> c2c_dist;
  std::array<DIST_TYPE,K> near_cls;

  CID_TYPE get_nearest_center(P_TYPE pnt, C_TYPE clusters[K], DIST_TYPE y_dist[K]) {
    DIST_TYPE min_dist = 0.0;
    CID_TYPE best_cl_id = pnt.id_cluster; 
    bool first_check = true; //only when elkan is off

  
		FIND_NEAR_CENT: for (int i=0; i<K; i++) {
		
		  DIST_TYPE dist = 0.0;

		  CALC_DIST: for (int j=0; j<DIM-1; j++) {
			dist += (pnt.coord[j] - clusters[i][j])*(pnt.coord[j] - clusters[i][j]);
		  }
		  dist += y_dist[i];
		  
		  if (dist < min_dist || first_check) {
			min_dist = dist;
			first_check=false;
			best_cl_id = (CID_TYPE)i;
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

  void center_to_center_dist(C_TYPE clusters[K]) {
    for (int i=0; i<K; i++) {
      DIST_TYPE min_dist=0;
      C_TYPE curr_cent = clusters[i];
      for (int j=0; j<K; j++) {
        if (i != j) {
          DIST_TYPE dist = 0;
          for (int k=0; k<DIM; k++) {
            dist += (curr_cent[k] - clusters[j][k])*(curr_cent[k] - clusters[j][k]);
          }
          if (min_dist == 0 || dist < min_dist) {
            min_dist = dist;
          }
          c2c_dist[i][j] = dist;
        }
      }
      near_cls[i] = min_dist;
    }
  };

public:
  KMEANS_SD() {};
  ~KMEANS_SD() {};
    
  #pragma hls_design interface  
  void CCS_BLOCK(run) (CSR_CRD_T points[N], CSR_POS_T pos[POS_SIZE], C_TYPE clusters_centers[K]) {


    int iter = 0;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {

      // CALCULATE CENTER TO CENTER DISTANCES
      center_to_center_dist(clusters_centers);
      
      keep_going = false;
    
      INIT_CNTR_ACC: for (int i=0; i<K; i++) {
        ac::init_array<AC_VAL_0>(&new_coord[i][0], DIM);
      }
      ac::init_array<AC_VAL_0>(&c_points[0], K);


      ASGN_TO_CLUST: for (int i=0; i<POS_SIZE-1; i++) {
          
        
        COORD_TYPE cur_y = pos[i].pointer;
        DIST_TYPE y_dist[K];
        COMMON_Y_DIST: for (int j=0; j<K; j++) {
          y_dist[j] = (clusters_centers[j][1] - cur_y)*(clusters_centers[j][1] -  cur_y);
        }
 
        ac_int<PID_BITS, false> l_start = pos[i].index;
        ac_int<PID_BITS, false> l_end = pos[i+1].index;
        
        
        ac_int<PID_BITS, false> num_pnts;
        ac_math::ac_abs(l_end-l_start, num_pnts); 
    
      
        PNT_OF_CRD: for (int l=0; l<num_pnts; l++) {
          ac_int<PID_BITS, false> k = l+l_start;
          P_TYPE curr_pnt;
          
          curr_pnt.coord[0] = points[k].coord[0];
          curr_pnt.coord[1] = cur_y;
          curr_pnt.id_cluster = points[k].id_cluster;

          CID_TYPE new_cl_id = get_nearest_center(curr_pnt, clusters_centers, y_dist);

          c_points[new_cl_id]++;
          ACC_NEW_CNTR: for (int j=0; j<DIM; j++) {
            new_coord[new_cl_id][j] += curr_pnt.coord[j];
          }

          if (new_cl_id != curr_pnt.id_cluster) {
            keep_going = true;
            points[k].id_cluster = new_cl_id;
          }
          

        }
          
      }

      update_cluster_center(clusters_centers);

      iter++;
    }    
    #ifndef __SYNTHESIS__
      std::cout << " FINISH AFTER : " << iter << " ITERATIONS!" << std::endl;
    #endif
  }; // end run
}; // end class
#endif
