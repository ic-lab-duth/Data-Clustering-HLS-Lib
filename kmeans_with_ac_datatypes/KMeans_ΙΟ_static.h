#ifndef K_MEANS_IO_STATIC_H 
#define K_MEANS_IO_STATIC_H

#include "datatypes_spec.h"
#include <ac_math.h>
#include <ac_channel.h>

#include <array>

template<int K, int N, int MAX_ITER>
class KMeans_IO {
private:
  std::array<P_TYPE,N> points;
  std::array<C_TYPE,K> clusters;

  void initialize_clusters() {
    for (int i = 0; i < K; i++) {
      C_TYPE cl;
      cl.id = (CLTID_TYPE)i;

      // int indx = rand()%(int)N_POINTS;
      int indx = i;

      cl.x = points[indx].x;
      cl.y = points[indx].y;
      
      clusters[i] = cl;
    }
  };

  DIST_TYPE calculate_dist(CLTID_TYPE cl_id, PNTID_TYPE pnt_id) {
    DIST_TYPE dist = (points[pnt_id].x - clusters[cl_id].x)*(points[pnt_id].x - clusters[cl_id].x) + 
                     (points[pnt_id].y - clusters[cl_id].y)*(points[pnt_id].y - clusters[cl_id].y);
    return dist;
  };
    
    
  COST_TYPE calculate_cost() {
    COST_TYPE cost = 0.0;
    for (int i=0; i<N; i++) {
      DIST_TYPE dist = calculate_dist(points[i].id_cluster, points[i].id);
      cost += dist;
    }
    return cost;
  };

  bool assign_points_to_clusters() {
    bool keep_going = false;
    for (int i=0; i<N; i++) {
      CLTID_TYPE curr_cl_id = points[i].id_cluster;
      CLTID_TYPE new_cl_id = get_nearest_center(points[i].id);

      if (new_cl_id != curr_cl_id) {
        keep_going = true;
        points[i].id_cluster = new_cl_id;
      }
    }
    return keep_going;
  };

  CLTID_TYPE get_nearest_center(PNTID_TYPE pnt_id) {
    DIST_TYPE min_dist = 0.0;
    CLTID_TYPE best_cl_id = points[pnt_id].id_cluster;
    DIST_TYPE dist = 0.0;
    bool first_check = true;
    for (int i=0; i<K; i++) {
      dist = calculate_dist(clusters[i].id, pnt_id);

      if (dist < min_dist || first_check) {
        min_dist = dist;
        best_cl_id = clusters[i].id;
        first_check = false;
      }
    }
    return best_cl_id;
  };
   

   void update_cluster_center() {
    std::array<CNT_ACC_TYPE,K> new_x;
    std::array<CNT_ACC_TYPE,K> new_y;

    std::array<COUNTER_TYPE,K> new_x_points;
    std::array<COUNTER_TYPE,K> c_points;
    
    for (int i=0; i<K; i++) {
      new_x[i] = 0.0;
      new_y[i] = 0.0;
      c_points[i] = 0;
    }

    for (int i=0; i<N; i++) {
      new_x[points[i].id_cluster] += points[i].x;
      new_y[points[i].id_cluster] += points[i].y;
      c_points[points[i].id_cluster] ++;
    }

    for (int i=0; i<K; i++) {
      clusters[i].x = new_x[i] / c_points[i];
      clusters[i].y = new_y[i] / c_points[i];
    }
  };

public:
  KMeans_IO() {};
  ~KMeans_IO() {};
    
    

  void run(ac_channel<P_TYPE> (&in_points)[N], ac_channel<CENTER_T> (&out_clusters)[K], ac_channel<CLTID_TYPE> (&out_p2c)[N]) {
    COST_TYPE old_cost = 0.0;
    COST_TYPE new_cost = 0.0;
    COST_TYPE diff = 0.0;

    // READ
    for (int i=0; i<N; i++) {
      points[i] = in_points[i].read();
      points[i].id = (PNTID_TYPE)i;
    }

    // DO JOB
    initialize_clusters();

    int iter = 0;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {
      keep_going = assign_points_to_clusters();
      update_cluster_center();

      new_cost = calculate_cost();
      if (old_cost > new_cost) {
        diff = old_cost - new_cost;
      } else {
        diff = new_cost - old_cost;
      }
      if (diff < 0.00001) {
          break;
      }
      old_cost = new_cost;
      iter++;
    }
    

    // WRITE
    CENTER_T tmp_center;
    for (int i=0; i<K; i++) {
      tmp_center.x = clusters[i].x;
      tmp_center.y = clusters[i].y;
      out_clusters[i].write(tmp_center);
    }
    
    CLTID_TYPE tmp_p2c;
    for (int i=0; i<N; i++) {
      tmp_p2c = points[i].id_cluster;
      out_p2c[i].write(tmp_p2c);
    }

  }; // end run
}; // end class
#endif



  // DEPR
  // void update_cluster_center() {
  //   CNT_ACC_TYPE new_x;
  //   CNT_ACC_TYPE new_y;
  //   for (auto& cl : clusters) {
  //     if (cl.points.size() == 0) {
  //       continue;
  //     }
  //     new_x = 0.0;
  //     new_y = 0.0;
  //     int w = 0;
  //     for (const auto& pnt_id : cl.points) {
  //       new_x += points[pnt_id].x;
  //       new_y += points[pnt_id].y;
  //       w++;
  //     }
  //     cl.x = new_x / cl.points.size();
  //     cl.y = new_y / cl.points.size();
  //     std::cout << w << std::endl;
  //   }
  //   std::cout << "\n" << std::endl;
  // };
