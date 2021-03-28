#ifndef MEAN_SHIFT_H
#define MEAN_SHIFT_H

#include "../datatypes_spec.h"
#include "../static_vector.h"
#include <mc_scverify.h>

#pragma hls_design
template<int N, int DIM, int MAX_ITER>
class MEAN_SHIFT {
private:
  
  std::array<ACCU_TYPE, DIM> new_coord;
  DIST_TYPE denominator;
  

  DIST_TYPE Gaussian_K(DIST_TYPE dist, DIST_TYPE rad) {
    DIST_TYPE nom = dist;
    DIST_TYPE denom = rad*rad;
    DIST_TYPE t = nom/denom;
       
    DIST_TYPE ret;
    ac_math::ac_exp_pwl(-t/2, ret);
    return ret;
  }

  DIST_TYPE Epanechnikov_K(DIST_TYPE dist, DIST_TYPE rad) {
    DIST_TYPE nom = dist;
    DIST_TYPE denom = rad*rad;
    DIST_TYPE t;
    ac_math::ac_div(nom, denom, t);
   
    return (t<1) ? (1-t) : 0;
  }

public:
  MEAN_SHIFT() {};
  ~MEAN_SHIFT() {};
    
  #pragma hls_design interface  
  void CCS_BLOCK(run) (P_TYPE points[N], C_TYPE clusters_centers[N], DIST_TYPE radius) {
    
    uniqueStack<DIM,N> clusters(clusters_centers);

    for (int i=0; i<N; i++) {
      C_TYPE current_cl;
      P_TYPE current_pnt = points[i];
      for (int j=0; j<DIM; j++) {
        current_cl[j] = current_pnt.coord[j];
      }
      clusters.push(current_cl);
    } 

    int iter = 0;
    bool keep_going = true;
    while (iter < MAX_ITER && keep_going) {
      
      keep_going = false;
      DIST_TYPE max_dist = 0.0;
      bool first_check = true;

      PID_TYPE bound = clusters.size();
      clusters.reset_pointer();
      
      FOR_EACH_CLS: for (int k=0; k<N; k++) {

          ac::init_array<AC_VAL_0>(&new_coord[0], DIM);
          denominator = 0;
        if (k < bound) {

          C_TYPE current = clusters.at((PID_TYPE)k);

          FOR_EACH_PNT: for (int i=0; i<N; i++) {
            P_TYPE cur_pnt = points[i];

            DIST_TYPE dist = 0;
            DIST_CALC: for (int j=0; j<DIM; j++) {
              dist += (current[j] - cur_pnt.coord[j])*(current[j] - cur_pnt.coord[j]);
            }
            
            DIST_TYPE kernel = Epanechnikov_K(dist, radius);
            
            ACCUM: for (int j=0; j<DIM; j++) {
              new_coord[j] += cur_pnt.coord[j] * kernel;
            }
            denominator += kernel;

          }
          
          C_TYPE new_center;
          DIST_TYPE dist = 0;
          ACCU_TYPE tmp;;
          UPD_CNT: for (int j=0; j<DIM; j++) {
            
            ac_math::ac_div(new_coord[j], denominator, new_center[j]);
            dist += (new_center[j] - current[j])*(new_center[j] - current[j]);
          }
          
          if (dist > max_dist || first_check) {
            max_dist = dist;
            first_check = false;
          }
          
          clusters.push_update(new_center, radius/(DIST_TYPE)4);
        }
      }

      
      if (max_dist > 0) { 
        keep_going = true;
      }
      
      iter++;
    }
    

    for (int i=0; i<N; i++) {
      P_TYPE curPnt = points[i];
      DIST_TYPE min_dist= 0;
      CID_TYPE best_cls_id;

      bool first_check = true;
      CID_TYPE cur_cid = 0;
      for (int j=0; j<N; j++) {
        if (j < clusters.size()) {
          C_TYPE curClust = clusters.at(j);
          DIST_TYPE dist = 0;
          for (int k=0; k<DIM; k++) {
            dist += (curPnt.coord[k]-curClust[k])*(curPnt.coord[k]-curClust[k]);
          }
          if (dist < min_dist || first_check) {
            min_dist = dist;
            best_cls_id = cur_cid;
            first_check = false;
          }
          cur_cid++;
        } else {
            break;
        }
      }
      points[i].id_cluster = best_cls_id;
    }
    
     
  }; // end run
}; // end class
#endif
