/* 
* Created by Dimitris Mangiras
*/
#include <fstream>
#include "KMeans_basic.h"

    KMeans_basic::KMeans_basic(int maximum_iterations,
                     int K_value, 
                     int maximum_cluster_size /* = std::numeric_limits<int>::max() */) {
        max_iters = maximum_iterations;
        K = K_value;
        max_cluster_size = maximum_cluster_size;
    }

    void KMeans_basic::add_data(std::vector<std::pair<float, float> > data) {
        for (auto pnt : data) {
            add_point(pnt.first, pnt.second);
        }
    }

    void KMeans_basic::initialize_clusters() {
        // if (K == 0) {
        //     K = (int)points.size()/max_cluster_size;
        //     if ((int)points.size()%max_cluster_size) {
        //         K++;
        //     }
        // }
        for (int i = 0; i < K; i++) {
            cluster_bs cl;
            cl.id = (int)clusters.size();
            // int indx = rand()%(int)points.size();
            int indx = i;
            cl.init_x = points[indx].x;
            cl.init_y = points[indx].y;
            cl.x = points[indx].x;
            cl.y = points[indx].y;
            
            clusters.push_back(cl);
        }
        assert((int)clusters.size() == K);
    }
        
    void KMeans_basic::run() {
        float old_cost = 0.0;
        float new_cost = 0.0;

        int iter = 1;
        bool keep_going = true;
        while ((iter <= max_iters) && keep_going) {
            keep_going = assign_points_to_clusters();
            update_cluster_center();

            new_cost = calculate_cost();
            if (std::fabs(new_cost - old_cost) < 1e-5) {
                break;
            }
            old_cost = new_cost;
            iter++;
        }
        std::cout << "END" << std::endl;
    }

    float KMeans_basic::calculate_cost() {
        float cost = 0.0;
        for (const auto& pnt : points) {
            assert(pnt.id_cluster != -1);
            float dist = calculate_dist(pnt.id_cluster, pnt.id);
            cost += dist;
        }
        return cost;
    }
    
    bool KMeans_basic::assign_points_to_clusters() {
        bool keep_going = false;
        for (auto& p : points) {
            int curr_cl_id = p.id_cluster;
            int new_cl_id = get_nearest_center(p.id);

            if (new_cl_id != curr_cl_id) {
                keep_going = true;
                if (curr_cl_id != -1) {
                    std::set<int>::iterator it = clusters[curr_cl_id].points.find(p.id);
                    assert(it != clusters[curr_cl_id].points.end());
                    clusters[curr_cl_id].points.erase(it);
                }
                clusters[new_cl_id].points.insert(p.id);
                p.id_cluster = new_cl_id;
            }
        }
        return keep_going;
    }

    int KMeans_basic::get_nearest_center(int pnt_id) {
        float min_dist = std::numeric_limits<float>::max();
        int best_cl_id = points[pnt_id].id_cluster;
        float dist = 0.0;
        for (const auto& cl : clusters) {
            if ((int)cl.points.size() >= max_cluster_size) {
                continue;
            }
            dist = calculate_dist(cl.id, pnt_id);

            if (dist < min_dist) {
                min_dist = dist;
                best_cl_id = cl.id;
            }
        }
        assert(best_cl_id != -1);
        return best_cl_id;
    }
    
    void KMeans_basic::update_cluster_center() {
        float new_x;
        float new_y;
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
    }

    float KMeans_basic::calculate_dist(int cl_id, int pnt_id) {
        return pow(points[pnt_id].x - clusters[cl_id].x, 2) + pow(points[pnt_id].y - clusters[cl_id].y, 2);
    }

    void KMeans_basic::add_point(float x, float y) {
        point_bs pnt;
        pnt.id = points.size();
        pnt.x = x;
        pnt.y = y;

        points.push_back(pnt);
    }

    int KMeans_basic::cluster_of_point(int pnt_id) {
        assert(pnt_id >= 0);
        assert(pnt_id < (int)points.size());

        return points[pnt_id].id_cluster;
    }

    int KMeans_basic::total_num_clusters() {
        return (int)clusters.size();
    }

    int KMeans_basic::get_cluster_size(int cl_id) {
        return (int)clusters[cl_id].points.size();
    }

    void KMeans_basic::print_clusters() { 
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
    }

    //DIONISIS
    std::vector<int> KMeans_basic::get_cluster_of_points() {
        std::vector<int> pnts;
        
        for (int i=0; i<points.size(); i++) {
          pnts.push_back(points[i].id_cluster);
        }
        return pnts;
    }

    std::vector<std::pair<float, float>> KMeans_basic::get_cluster_centers() {
        std::vector<std::pair<float, float>> centers;
        std::pair<float, float> tmp_pair;

        for (int i=0; i<K; i++) {
            tmp_pair.first = clusters[i].x;
            tmp_pair.second = clusters[i].y;
            centers.push_back(tmp_pair);
        }
        return centers;
    }

