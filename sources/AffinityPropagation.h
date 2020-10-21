/* 
* Created by Dimitris Mangiras
*/
#ifndef AFFINITY_PROPAGATION_H
#define AFFINITY_PROPAGATION_H

#include <string>
#include <assert.h>
#include <time.h>
#include <set>
#include <map>
#include <vector>
#include <math.h>
#include <chrono>
#include <stdlib.h>
#include <iostream>

// #include <time.h>
// #include <chrono>
// #include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>

#define MAX_FLOAT std::numeric_limits<float>::max()
#define MAX_INT std::numeric_limits<int>::max()

    enum Type {
        SIMILARITY = 0, RESPONSIBILITY = 1, AVAILABILITY = 2, ALL_TYPES = 3
    };

    struct Node {
        std::string name;
        int id;
        int id_cluster;

        float x;
        float y;
        float lat;

        float max_criterion;
        float sum_of_neighs_responsibilities;

        std::set<int> neighs;
        std::map<int, float> messages[Type::ALL_TYPES];

        std::vector<std::pair<int, float> > max_similarity;
        std::vector<std::pair<int, float> > max_resp;

        Node() : name(""), id(-1), id_cluster(-1), x(0.0), y(0.0), lat(0.0), max_criterion(-MAX_FLOAT), sum_of_neighs_responsibilities(0.0) {}
    };

    class Affinity_Propagation {
      public:
        std::vector<Node> nodes;
        int pnts_sz = 0;

        std::string data_name;

        int max_iters = 10;
        int conv_iters = 5;
        float lambda = 0.5;
        float max_coord;

        std::map<int, std::set<int> > clusters;
        std::vector<int> centers;

      public:
        Affinity_Propagation();

        void add_data(std::vector<std::pair<float, float> > data);
        void add_node(float x, float y);
        void prepare();
        void run();
        void calculate_responsibility();
        void calculate_availability();
        void calculate_criterion();
        void extract_clusters();
        void set_data_name(std::string value) {
            data_name = value;
        }
        void print_matrix(int node_id);
        void print_clusters(bool show_lines = false);
        void print_clusters_v2();
        bool keep_going(int& unchanged_centers);
        void normalize_xy_info();

        void set_max_iters(int value) {
            max_iters = value;
        }
        void set_lambda(float value) {
            lambda = value;
        }
        float get_max_coord() {
            return max_coord;
        }
        float find_median(std::vector<float> dists);

        // for evaluation see 
        // https://towardsdatascience.com/unsupervised-machine-learning-affinity-propagation-algorithm-explained-d1fef85f22c8
        void test();
    };

#endif