#ifndef K_MEANS_DATATYPES_H 
#define K_MEANS_DATATYPES_H

#include "datatypes_spec.h"
#include <ac_math.h>

#include <assert.h>
#include <time.h>
#include <set>
#include <vector>
#include <math.h>
#include <chrono>
#include <stdlib.h>
#include <iostream>

template<int K, int MAX_ITER>
class KMeans_datatypes {
private:
  std::vector<C_TYPE> clusters;
  std::vector<P_TYPE> points;

  std::string data_name; // only used for data printing (will be removed)

  void initialize_clusters() {
    for (int i = 0; i < K; i++) {
      C_TYPE cl;
      cl.id = (CLTID_TYPE)clusters.size();
      PNTID_TYPE indx = rand()%(PNTID_TYPE)points.size();

      cl.x = points[indx].x;
      cl.y = points[indx].y;
      
      clusters.push_back(cl);
    }
  };

  DIST_TYPE calculate_dist(CLTID_TYPE cl_id, PNTID_TYPE pnt_id) {
    DIST_TYPE dist = (points[pnt_id].x - clusters[cl_id].x)*(points[pnt_id].x - clusters[cl_id].x) + 
                     (points[pnt_id].y - clusters[cl_id].y)*(points[pnt_id].y - clusters[cl_id].y);
    return dist;
  };
    
    
  COST_TYPE calculate_cost() {
    COST_TYPE cost = 0.0;
    for (const auto& pnt : points) {
      DIST_TYPE dist = calculate_dist(pnt.id_cluster, pnt.id);
      cost += dist;
    }
    return cost;
  };

  bool assign_points_to_clusters() {
    bool keep_going = false;
    for (auto& p : points) {
      CLTID_TYPE curr_cl_id = p.id_cluster;
      CLTID_TYPE new_cl_id = get_nearest_center(p.id);

      if (new_cl_id != curr_cl_id) {
        keep_going = true;
        clusters[new_cl_id].points.insert(p.id);
        p.id_cluster = new_cl_id;
      }
    }
    return keep_going;
  };

  CLTID_TYPE get_nearest_center(PNTID_TYPE pnt_id) {
    DIST_TYPE min_dist = 0.0;
    CLTID_TYPE best_cl_id = points[pnt_id].id_cluster;
    DIST_TYPE dist = 0.0;
    for (const auto& cl : clusters) {
      dist = calculate_dist(cl.id, pnt_id);

      if (dist < min_dist || min_dist == 0.0) {
        min_dist = dist;
        best_cl_id = cl.id;
      }
    }
    return best_cl_id;
  };

  void update_cluster_center() {
    CNT_ACC_TYPE new_x;
    CNT_ACC_TYPE new_y;
    for (auto& cl : clusters) {
      if (cl.points.size() == 0) {
        continue;
      }
      new_x = 0.0;
      new_y = 0.0;
      for (const auto& pnt_id : cl.points) {
        new_x += points[pnt_id].x;
        new_y += points[pnt_id].y;
      }
      cl.x = new_x / cl.points.size();
      cl.y = new_y / cl.points.size();
    }
  };

public:
  KMeans_datatypes() {};
  ~KMeans_datatypes() {};
    
    

  void run() {
    COST_TYPE old_cost = 0.0;
    COST_TYPE new_cost = 0.0;
    COST_TYPE diff;

    initialize_clusters();

    // std::cout << "INITIALIZE CLUSTERS: DONE" << std::endl;

    int iter = 1;
    bool keep_going = true;
    while ((iter <= MAX_ITER) && keep_going) {
      keep_going = assign_points_to_clusters();
      // std::cout << "ASSIGN PNT TO CLST: DONE" << std::endl;
      update_cluster_center();
      // std::cout << "UPDATE CLUSTER CENTER: DONE" << std::endl;

      new_cost = calculate_cost();
      // std::cout << "COST CALCULATION: DONE" << std::endl;
      if (old_cost > new_cost) {
        diff = old_cost - new_cost;
      } else {
        diff = new_cost - old_cost;
      }
      if (diff < 0.0005) {
          break;
      }
      old_cost = new_cost;
      iter++;
    }
  };

    

    




    // INPUT & PRINT DATA FUNCTIONS --- NEED COMPLETE CHANGE FOR HLS
   
    
    std::vector<CLTID_TYPE> get_cluster_of_points() {
        std::vector<CLTID_TYPE> pnts;
        
        for (int i=0; i<points.size(); i++) {
          pnts.push_back(points[i].id_cluster);
        }
        return pnts;
    }

    std::vector<std::pair<COORD_TYPE, COORD_TYPE>> get_cluster_centers() {
        std::vector<std::pair<COORD_TYPE, COORD_TYPE>> centers;
        std::pair<COORD_TYPE, COORD_TYPE> tmp_pair;

        for (int i=0; i<K; i++) {
            tmp_pair.first = clusters[i].x;
            tmp_pair.second = clusters[i].y;
            centers.push_back(tmp_pair);
        }
        return centers;
    }









    void set_data_name(std::string value) {
        data_name = value;
    };

    void add_data(std::vector<std::pair<COORD_TYPE, COORD_TYPE> > data) {
      for (auto pnt : data) {
        add_point(pnt.first, pnt.second);
      }
    };

    void add_point(COORD_TYPE x, COORD_TYPE y) {
      P_TYPE pnt;
      pnt.id = (PNTID_TYPE)points.size();
      pnt.x = x;
      pnt.y = y;

      points.push_back(pnt);
    };
    
    void print_clusters() { 
    //writes a matlab file
    std::ofstream txt(data_name + "_kmeans_basic.m");
    txt << "clear; clc;" << std::endl;
    txt << "close all;" << std::endl << std::endl;

    txt << "colorspec = {'r'; 'g'; 'b'; 'c'; 'm'; [0.9290 0.6940 0.1250]; [0.4660 0.6740 0.1880]; [0.6350 0.0780 0.1840]};" << std::endl << std::endl;

    int cl_num = 0;
    for (const auto& cl : clusters) {
        if (cl.points.size() == 0) {
            continue;
        }
        txt << std::endl << "%---- " << cl_num << " ----" << std::endl;
        txt << "x" << cl_num << " = [ ";
        for (const auto& pnt_id : cl.points) {
            txt << points[pnt_id].x << " ";
        }
        txt << "];" << std::endl;

        txt << "y" << cl_num << " = [ ";
        for (const auto& pnt_id : cl.points) {
            txt << points[pnt_id].y << " ";
        }
        txt << "];" << std::endl;

        if (cl_num) {
            txt << "hold on;" << std::endl;
        }
        // txt << "plot(x" << cl_num << ", y" << cl_num << ", 'Color', colorspec{" << 1+(cl_num%8) << "}, 'LineStyle', '.');" << std::endl;
        txt << "plot(x" << cl_num << ", y" << cl_num << ", '.', 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl;

        txt << std::endl;
        txt << "hold on;" << std::endl;
        // txt << "plot(" << cl.x << ", " << cl.y << ", 'Color', colorspec{" << 1+(cl_num%8) << "}, 'LineStyle', 'x');" << std::endl << std::endl; 
        // txt << "plot(" << cl.x << ", " << cl.y << ", 'x', 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl << std::endl; 
        txt << "plot(" << cl.x << ", " << cl.y << ", 'xk');" << std::endl << std::endl; 
        cl_num++;
    }
    txt << "title('k-Means')" << std::endl;
    txt.close();
};

}; // end class
#endif


// DEPRECATED
/*

int get_cluster_size(CLTID_TYPE cl_id) {
  return (int)clusters[cl_id].points.size();
};

int total_num_clusters() {
  return (int)clusters.size();
};

CLTID_TYPE cluster_of_point(PNTID_TYPE pnt_id) {
  assert(pnt_id >= 0);
  assert(pnt_id < (int)points.size());

  return points[pnt_id].id_cluster;
};


*/
