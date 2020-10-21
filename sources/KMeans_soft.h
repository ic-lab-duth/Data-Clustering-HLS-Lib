/* 
* Created by Dimitris Mangiras
*/
#ifndef K_MEANS_SOFT_H 
#define K_MEANS_SOFT_H

#include <assert.h>
#include <time.h>
#include <set>
#include <vector>
#include <math.h>
#include <chrono>
#include <stdlib.h>
#include <iostream>

namespace soft {
    struct point {
        int id;          // id of the point
        int id_cluster;  // id of the cluster that this point belongs to

        float x;
        float y;

        float weight;

        point() : id(-1), id_cluster(-1), x(0.0), y(0.0), weight(0.0) {}
    };

    struct cluster {
        int id;
        
        float x;
        float y;
        float init_x;
        float init_y;

        std::set<int> points;

        cluster() : id(-1), x(0.0), y(0.0), init_x(0.0), init_y(0.0) {}
    };


    class KMeans_soft {
      private:
        std::vector<cluster> clusters;
        std::vector<point> points;

        std::vector<std::vector<float> > membership; //points x clusters

        std::string data_name;
        int max_iters = 100;
        int max_cluster_size = std::numeric_limits<int>::max();
        int K = 0;
        float max_coord = -std::numeric_limits<float>::max();

        // soft KMeans variables
        float p = 3;

      public:
        KMeans_soft(int maximum_iterations, 
                     int K_value, 
                     int maximum_cluster_size = std::numeric_limits<int>::max());
        
        void set_data_name(std::string value) {
            data_name = value;
        }
        void add_data(std::vector<std::pair<float, float>> data);
        void initialize_clusters();
        void run();
        float calculate_cost();
        void calculate_memberships_and_weights();
        void assign_points_to_clusters();
        int get_better_center(int pnt_id);
        void update_cluster_centers();
        bool has_converged(float prev_cost, float new_cost);

        float calculate_dist(int cl_id, int pnt_id);

        void add_point(float x, float y);
        int cluster_of_point(int pnt_id);
        int total_num_clusters();
        int get_cluster_size(int cl_id);
        void print_clusters();

    }; // end class

}
#endif