#ifndef DBSCAN_H
#define DBSCAN_H

#include "../datatypes_spec.h"
#include "../static_vector.h"
#include <mc_scverify.h>

#pragma hls_design
template<int N, int DIM>
class DBSCAN {
private:
  

  PID_TYPE find_neighbors(P_TYPE pnts[N], bool pnt_type[N], PID_TYPE index, uniqueQueue<DIM,N> &index_Q, ac_fixed<10,1,false> eps, bool secondN) {
    
    PID_TYPE neighbors = 0;
    P_TYPE cur_pnt = pnts[index];
    FIND_NEIGH: for (int i=0; i<N; i++) {
      if ((PID_TYPE)i != index) {

        DIST_TYPE dist = 0;
        for (int j=0; j<DIM; j++) {
          dist += (pnts[i].coord[j]-cur_pnt.coord[j])*(pnts[i].coord[j]-cur_pnt.coord[j]);
        }
        if (dist <= eps) {
          neighbors++;
          if (secondN)
            index_Q.push_unique((PID_TYPE)i);
          else
            index_Q.push((PID_TYPE)i);   
        }
      }
    }

    return neighbors;
  }



public:
  DBSCAN() {};
  ~DBSCAN() {};
    
  #pragma hls_design interface  
  void CCS_BLOCK(run) (P_TYPE points[N], bool pnt_type[N], PID_TYPE index_queue[N], int minPts, ac_fixed<10,1,false> eps) {

    uniqueQueue<DIM,N> neighborsQ(index_queue);
    
    INIT: for (int i=0; i<N; i++) {
      pnt_type[i] = false;
      points[i].id_cluster = 0;
    }


    PID_TYPE cluster_id = 0;
    OUTER: for (int i=0; i<N; i++) {
     // bool unclassified = !pnt_type[i]; // type=false : unclassified  |  type=true : Noise or Assigned
      if (pnt_type[i] == false) {
        PID_TYPE cur_index = (PID_TYPE)i;

        neighborsQ.reset_queue();

        //neighborsQ.push(cur_index);
        PID_TYPE neighbors = find_neighbors(points, pnt_type, cur_index, neighborsQ, eps, false);
        if (neighbors >= (minPts-1)) {
          #ifndef __SYNTHESIS__
            std::cout << neighbors << std::endl;
          #endif 
          cluster_id++;
          points[cur_index].id_cluster = cluster_id;
          pnt_type[cur_index] = true;
          
          INNER: for (int j=0; j<N; j++) {
            PID_TYPE Qsize = neighborsQ.size();
            if (j < Qsize) {
              PID_TYPE index = index_queue[j];

              points[index].id_cluster = cluster_id;
              if (pnt_type[index]==false)
              pnt_type[index] = true;
              
              PID_TYPE second_neighb = find_neighbors(points, pnt_type, index, neighborsQ, eps, true);
              if (second_neighb < (minPts-1)) {
                neighborsQ.resize_Q(Qsize);
              }
            } else {
              break;
            }
          }
        } else {
          pnt_type[cur_index] = true;  // set as noise
        }
      }
      
    }
    
    #ifndef __SYNTHESIS__
    std::cout << cluster_id << std::endl;
    #endif
         
  }; // end run
}; // end class
#endif
