/* 
* Created by Dimitris Mangiras
*/
#include <fstream>
#include "KMeans_soft.h"

namespace soft {
    KMeans_soft::KMeans_soft(int maximum_iterations,
                     int K_value, 
                     int maximum_cluster_size /* = std::numeric_limits<int>::max() */) {
        max_iters = maximum_iterations;
        K = K_value;
        max_cluster_size = maximum_cluster_size;
    }

    void KMeans_soft::add_data(std::vector<std::pair<float, float> > data) {
        for (auto pnt : data) {
            add_point(pnt.first, pnt.second);
        }
    }

    void KMeans_soft::initialize_clusters() {
        //normalization
        for (auto& pnt : points) {
            pnt.x = pnt.x / max_coord;
            pnt.y = pnt.y / max_coord;
        }

        if (K == 0) {
            K = (int)points.size()/max_cluster_size;
            if ((int)points.size()%max_cluster_size) {
                K++;
            }
        }

        for (int i = 0; i < K; i++) {
            cluster cl;
            cl.id = (int)clusters.size();
            int indx = rand()%(int)points.size();
            cl.init_x = points[indx].x;
            cl.init_y = points[indx].y;
            cl.x = points[indx].x;
            cl.y = points[indx].y;
            
            clusters.push_back(cl);
        }
        assert((int)clusters.size() == K);
    }
        
    void KMeans_soft::run() {
        membership = std::vector<std::vector<float> >(points.size(), std::vector<float>(clusters.size(), 0.0));
        int iter = 1;
        bool keep_going = true;
        float prev_cost = calculate_cost();
        float new_cost;
        while ((iter <= max_iters) && keep_going) {
            calculate_memberships_and_weights();
            update_cluster_centers();
            
            new_cost = calculate_cost();
            keep_going = !has_converged(prev_cost, new_cost);
            prev_cost = new_cost;
            // std::cout << "---- " << iter << " ----" << std::endl;
            iter++;
        }
        assign_points_to_clusters();
    }

    float KMeans_soft::calculate_cost() {
        float cost = 0.0;
        for (const auto& pnt : points) {
            float sum = 0.0;
            for (const auto& cl : clusters) {
                float dist = calculate_dist(cl.id, pnt.id);
                dist = pow(dist, p);
                if (dist < 1e-5) {
                    continue;
                }
                sum += (1.0 / dist);
            }
            if (sum < 1e-5) {
                continue;
            }
            cost += (K / sum);
        }
        return cost;
    }

    void KMeans_soft::calculate_memberships_and_weights() {
        for (int i = 0; i < (int)points.size(); i++) {
            float sum = 0.0;
            float sum2 = 0.0;
            for (int j = 0; j < (int)clusters.size(); j++) {
                float dist = calculate_dist(j, i);
                if (dist == 0.0) {
                    dist = 1e-2;
                }
                membership[i][j] = pow(dist, -p-2);

                if (isnan(membership[i][j])) {
                    std::cout << "membership(i,j) = (" << i << "," << j << ") - is nan" << std::endl;
                    assert(false);
                }

                if (isinf(membership[i][j])) {
                    std::cout << "membership(i,j) = (" << i << "," << j << ") - is inf" << std::endl;
                    std::cout << "dist = " << dist << " , pow(dist,-p-2) = " << membership[i][j] << std::endl;
                    assert(false);
                }

                sum  += membership[i][j];
                sum2 += pow(dist, -p);
            }

            float old_value;
            for (auto& value : membership[i]) {
                // practically zero
                if (sum < 1e-5) {
                    break;
                }
                old_value = value;
                value = value / sum;

                if (isnan(value)) {
                    std::cout << "value of point " << i << " - is nan" << std::endl;
                    std::cout << "prev val = " << old_value << " , sum = " << sum << std::endl;
                    assert(false);
                }
            }
            if (sum2 < 1e-5) {
                // practically zero
                points[i].weight = 0.0;    
            } else {
                points[i].weight = sum / pow(sum2, 2);
            }
        }
    }
    
    void KMeans_soft::assign_points_to_clusters() {
        for (auto& p : points) {
            int new_cl_id = get_better_center(p.id);

            clusters[new_cl_id].points.insert(p.id);
            p.id_cluster = new_cl_id;
        }
    }

    int KMeans_soft::get_better_center(int pnt_id) {
        float max_membership = -std::numeric_limits<float>::max();
        int best_cl_id = -1;
        for (int j = 0; j < (int)membership[pnt_id].size(); j++) {
            if (membership[pnt_id][j] > max_membership) {
                max_membership = membership[pnt_id][j];
                best_cl_id = j;
            }
        }
        assert(best_cl_id != -1);
        return best_cl_id;
    }
    
    void KMeans_soft::update_cluster_centers() {
        float new_x;
        float new_y;
        float sum;
        float prod;
        for (auto& cl : clusters) {
            new_x = 0.0;
            new_y = 0.0;
            sum = 0.0;
            prod = 0.0;
            for (const auto& pnt : points) {
                prod = membership[pnt.id][cl.id] * pnt.weight;
                new_x += prod * pnt.x;
                new_y += prod * pnt.y;
                sum   += prod; 
            }
            if (sum < 1e-5) {
                cl.x = 0.0;
                cl.y = 0.0;
            } else {
                cl.x = new_x / sum;
                cl.y = new_y / sum;
            }
        }
    }

    bool KMeans_soft::has_converged(float prev_cost, float new_cost) {
        if (std::fabs(prev_cost - new_cost) < 1e-5) {
            return true;
        }
        return false;
    }

    float KMeans_soft::calculate_dist(int cl_id, int pnt_id) {
        return std::sqrt(pow(points[pnt_id].x - clusters[cl_id].x, 2) + pow(points[pnt_id].y - clusters[cl_id].y, 2));
    }

    void KMeans_soft::add_point(float x, float y) {
        point pnt;
        pnt.id = points.size();
        pnt.x = x;
        pnt.y = y;

        //for normalization
        max_coord = std::max(max_coord, x);
        max_coord = std::max(max_coord, y);

        points.push_back(pnt);
    }

    int KMeans_soft::cluster_of_point(int pnt_id) {
        assert(pnt_id >= 0);
        assert(pnt_id < (int)points.size());

        return points[pnt_id].id_cluster;
    }
 
    int KMeans_soft::total_num_clusters() {
        return (int)clusters.size();
    }

    int KMeans_soft::get_cluster_size(int cl_id) {
        return (int)clusters[cl_id].points.size();
    }

    void KMeans_soft::print_clusters() { 
        //writes a matlab file
        std::ofstream txt(data_name + "_kmeans_soft.m");
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
                txt << points[pnt_id].x*max_coord << " ";
            }
            txt << "];" << std::endl;

            txt << "y" << cl_num << " = [ ";
            for (const auto& pnt_id : cl.points) {
                txt << points[pnt_id].y*max_coord << " ";
            }
            txt << "];" << std::endl;

            if (cl_num) {
                txt << "hold on;" << std::endl;
            }
            // txt << "plot(x" << cl_num << ", y" << cl_num << ", 'Color', colorspec{" << 1+(cl_num%8) << "}, 'LineStyle', '.');" << std::endl;
            txt << "plot(x" << cl_num << ", y" << cl_num << ", '.', 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl;

            txt << std::endl;
            txt << "hold on;" << std::endl;
            // txt << "plot(" << cl.x*max_coord << ", " << cl.y*max_coord << ", 'Color', colorspec{" << 1+(cl_num%8) << "}, 'LineStyle', 'x');" << std::endl << std::endl; 
            txt << "plot(" << cl.x*max_coord << ", " << cl.y*max_coord << ", 'x', 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl << std::endl; 
            cl_num++;
        }
        txt << "title('kHM-soft')" << std::endl;
        txt.close();
    }
}