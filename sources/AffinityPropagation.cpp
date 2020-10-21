#include "AffinityPropagation.h"

    bool sortByDistance_2(const std::pair<int, float> &a, const std::pair<int, float> &b) { 
        return (a.second < b.second); 
    } 

    Affinity_Propagation::Affinity_Propagation() {
        max_coord = -MAX_FLOAT;

        clusters.clear();
        nodes.clear();
    }

    void Affinity_Propagation::add_data(std::vector<std::pair<float, float> > data) {
        for (auto pnt : data) {
            add_node(pnt.first, pnt.second);
        }
    }

    void Affinity_Propagation::add_node(float x, float y) {
        Node node;
        node.id = nodes.size();
        node.x = x;
        node.y = y;

        max_coord = std::max(max_coord, (float)node.x);
        max_coord = std::max(max_coord, (float)node.y);
        nodes.push_back(node);
        pnts_sz = nodes.size();
    }

    float Affinity_Propagation::find_median(std::vector<float> dists) {
        // First we sort the array 
        std::sort(dists.begin(), dists.end()); 
      
        int n = dists.size();
        // check for even case 
        if (n % 2 != 0) 
           return dists[n/2]; 
          
        return (dists[(n-1)/2] + dists[n/2])/2.0; 
    }

    void Affinity_Propagation::prepare() {
        float min_dist = std::numeric_limits<float>::max();
        float max_dist = MAX_FLOAT;

        int total_nums = 0;
        bool use_median = true;
        int max_neighs = 1000;
        max_neighs = std::min(max_neighs, (int)nodes.size());
        // std::cout << "Max neighs = " << max_neighs << std::endl;
        bool full_connectivity = (max_neighs == (int)nodes.size()) ? true : false;

        std::vector<float> dists_for_median;
        auto START = std::chrono::high_resolution_clock::now();
        if (full_connectivity) {
            std::cout << "Full connectivity" << std::endl;
            for (int i = 0; i < (int)nodes.size(); i++) {
                nodes[i].neighs.insert(i);
                for (int j = i+1; j < (int)nodes.size(); j++) {
                    float dist = std::pow((nodes[i].x - nodes[j].x), 2) + std::pow((nodes[i].y - nodes[j].y), 2);
                    if (dist > max_dist) {
                        continue;
                    }
                    nodes[i].neighs.insert(j);
                    nodes[i].messages[Type::SIMILARITY][j] = -dist;
              
                    nodes[j].neighs.insert(i);
                    nodes[j].messages[Type::SIMILARITY][i] = -dist;

                    if (use_median) {
                        dists_for_median.push_back(-dist);
                    } else {
                        min_dist = std::min(-dist, min_dist);
                    }
                    total_nums++;
                }
            }
        } else {
            for (auto& n : nodes) {
                std::vector<std::pair<int, float>> vec;
                for (const auto& n2 : nodes) {
                    float dist = std::sqrt(std::pow((n.x - n2.x), 2) + std::pow((n.y - n2.y), 2));
                    vec.push_back(std::make_pair(n2.id, dist));
                }
                std::sort(vec.begin(), vec.end(), sortByDistance_2);

                for (int i = 0; i < max_neighs; i++) {
                    n.neighs.insert(vec[i].first);
                    n.messages[Type::SIMILARITY][vec[i].first] = -vec[i].second;
                    if (use_median) {
                        dists_for_median.push_back(-vec[i].second);
                    } else {
                        min_dist = std::min(min_dist, -vec[i].second);
                    }
                    total_nums++;
                }
                assert(n.neighs.size() > 1);
                assert((int)n.neighs.size() == max_neighs);
                assert((int)n.messages[Type::SIMILARITY].size() == max_neighs);
            }

        }

        if (use_median) {
            min_dist = find_median(dists_for_median);
        }

        // std::cout << "Min dist found (use median ? " << use_median << ") = " << min_dist << std::endl;
        // std::cout << "Similarity matrix non-zero elements = " << total_nums << std::endl;

        // std::cout << "ok 3" << std::endl;
        for (int i = 0; i < (int)nodes.size(); i++) {
            nodes[i].messages[Type::SIMILARITY][i] = min_dist;
        }
        // std::cout << "ok 4" << std::endl;


        for (const auto& n : nodes) {
            if ((int)n.neighs.size() != max_neighs) {
                std::cout << "Node " << n.id << " has " << n.neighs.size() << " neighbohrs" << std::endl;
                assert(false);
            }
            if ((int)n.messages[Type::SIMILARITY].size() != max_neighs) {
                std::cout << "Node " << n.id << " has " << n.messages[Type::SIMILARITY].size() << " SIMILARITY neighbohrs" << std::endl;
                assert(false);
            }
            for (const auto& elem : n.messages[Type::SIMILARITY]) {
                if (elem.second > 0) {
                    std::cout << "Node " << n.id << " with neighbor " << elem.first << " has non-negative Similarity = " << elem.second << std::endl;
                    assert(false);
                }
            }
        }

        auto END = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> DURATION = END - START;
        std::cout << "Duration of preparation =  " << DURATION.count() << std::endl;
    }

    void Affinity_Propagation::run() {
        assert(pnts_sz == (int)nodes.size());
        auto start = std::chrono::high_resolution_clock::now();
        
        auto start_step = std::chrono::high_resolution_clock::now();
        auto end_step = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration_step = end_step - start_step;

        bool cont = true;
        int iter = 1;
        int unchanged_centers = 0;
        while (cont && (iter <= max_iters)) {
            std::cout << "--- iter " << iter << " ---" << std::endl;
            
            start_step = std::chrono::high_resolution_clock::now();
            calculate_responsibility();
            end_step = std::chrono::high_resolution_clock::now();
            duration_step = end_step - start_step;
            std::cout << "resp... ok - " << duration_step.count() << std::endl;
            
            start_step = std::chrono::high_resolution_clock::now();
            calculate_availability();
            end_step = std::chrono::high_resolution_clock::now();
            duration_step = end_step - start_step;
            std::cout << "avail... ok - " << duration_step.count() << std::endl;

            cont = keep_going(unchanged_centers);
            iter++;
        }

        // start_step = std::chrono::high_resolution_clock::now();
        // calculate_criterion();
        // end_step = std::chrono::high_resolution_clock::now();
        // duration_step = end_step - start_step;
        // std::cout << "crit... ok - " << duration_step.count() << std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Duration of run =  " << duration.count() << std::endl;
    }
    
    void Affinity_Propagation::calculate_responsibility() {
        for (auto& n : nodes) {
            n.max_similarity.clear();

            float maxA = -MAX_FLOAT;
            float maxB = -MAX_FLOAT;

            int indxA = -1;
            int indxB = -1;
            for (const auto& elem : n.messages[Type::SIMILARITY]) {
                int neigh_id = elem.first;
                float value = elem.second + n.messages[Type::AVAILABILITY][neigh_id];
                if (value > maxA) {
                    maxB = maxA;
                    maxA = value;
                    indxA = neigh_id;
                    indxB = indxA;
                } else if (value > maxB) {
                    maxB = value;
                    indxB = neigh_id;
                }
            }
            n.max_similarity.push_back(std::make_pair(indxA, maxA));
            n.max_similarity.push_back(std::make_pair(indxB, maxB));
            assert(n.max_similarity.size() == 2);
        }

        for (auto& n : nodes) {
            for (const auto& neigh : n.neighs) {
                float max_val = (neigh == n.max_similarity[0].first) ? n.max_similarity[1].second : n.max_similarity[0].second;
                n.messages[Type::RESPONSIBILITY][neigh] = lambda*n.messages[Type::RESPONSIBILITY][neigh] + (1 - lambda)*(n.messages[Type::SIMILARITY][neigh] - max_val);
            }
        }
    }

    void Affinity_Propagation::calculate_availability() {
        for (auto& n : nodes) {
            float sum = 0.0;
            n.sum_of_neighs_responsibilities = 0.0;
            for (const auto& neigh : n.neighs) {
                n.sum_of_neighs_responsibilities += std::max((float)0, nodes[neigh].messages[Type::RESPONSIBILITY][n.id]);
                if (neigh == n.id) {
                    continue;
                }
                sum = sum + std::max((float)0, (float)nodes[neigh].messages[Type::RESPONSIBILITY][n.id]);
            }
            n.messages[Type::AVAILABILITY][n.id] = lambda*n.messages[Type::AVAILABILITY][n.id] + (1 - lambda)*sum;
        }
        std::cout << "self-availability... ok" << std::endl;

        for (auto& n : nodes) {
            for (const auto& neigh : n.neighs) {
                if (neigh == n.id) {
                    continue;
                }
                float final_val = nodes[neigh].messages[Type::RESPONSIBILITY][neigh] + nodes[neigh].sum_of_neighs_responsibilities;
                final_val -= std::max((float)0, nodes[n.id].messages[Type::RESPONSIBILITY][neigh]);
                final_val -= std::max((float)0, nodes[neigh].messages[Type::RESPONSIBILITY][neigh]);
                // for (const auto& neigh_neigh : nodes[neigh].neighs) {
                //     if ((neigh_neigh == n.id) || (neigh_neigh == neigh)) {
                //         final_val -= std::max((float)0, nodes[neigh_neigh].messages[Type::RESPONSIBILITY][neigh]);
                //     }
                // }
                n.messages[Type::AVAILABILITY][neigh] = lambda*n.messages[Type::AVAILABILITY][neigh] + (1 - lambda)*std::min((float)0, final_val);
            }
        }
    }

    void Affinity_Propagation::calculate_criterion() {
        assert(false);
        // for (auto& n : nodes) {
        //     for (const auto& neigh : n.neighs) {
        //         n.messages[CRITERION][neigh] = n.messages[Type::RESPONSIBILITY][neigh] + n.messages[Type::AVAILABILITY][neigh];
        //     }
        // }
    }

    bool Affinity_Propagation::keep_going(int& unchanged_centers) {
        std::vector<int> old_centers = centers;
        bool cont = false;
        centers.clear();
        for (const auto& n : nodes) {
            if ((n.messages[Type::RESPONSIBILITY].at(n.id) + n.messages[Type::AVAILABILITY].at(n.id)) > 0) {
                centers.push_back(n.id);
                std::vector<int>::iterator it = find(old_centers.begin(), old_centers.end(), n.id);
                if (it == old_centers.end()) {
                    //new exampler
                    cont = true;
                }
            }
        }

        if (!cont && (centers.size() == old_centers.size())) {
            unchanged_centers++;
        } else {
            unchanged_centers = 0;
        }
        return !(unchanged_centers == conv_iters);
    } 

    void Affinity_Propagation::normalize_xy_info() {
        assert(max_coord != MAX_FLOAT);

        if (max_coord == 0.0) {
            std::cout << "Max coord = 0.0" << std::endl;
            return;
        }

        float max_xy = -MAX_FLOAT;
        for (auto& n : nodes) {
            n.x = n.x / max_coord;
            n.y = n.y / max_coord;

            max_xy = std::max(max_xy, n.x);
            max_xy = std::max(max_xy, n.y);
        }
        assert(max_xy == 1.0);
    }

    void Affinity_Propagation::extract_clusters() {
        int no_center_neighbor = 0;
        clusters.clear();
        centers.clear();
        for (auto& n : nodes) {
            n.id_cluster = -1;
            if ((n.messages[Type::RESPONSIBILITY][n.id] + n.messages[Type::AVAILABILITY][n.id]) > 0) {
                centers.push_back(n.id);
                n.id_cluster = n.id;
                clusters[n.id].insert(n.id);
            }
        }
        std::cout << "clusters - ok1 #centers = " << centers.size() << std::endl;

        for (auto& n : nodes) {
            // std::cout << "node " << n.id << std::endl;
            if (n.id_cluster != -1) {
                //this point is an exampler
                continue;
            }
            float max_similarity = -MAX_FLOAT;
            int max_sim_exampler = -1;
            for (const auto& c : centers) {
                std::map<int, float>::iterator it = n.messages[Type::SIMILARITY].find(c);
                if (it == n.messages[Type::SIMILARITY].end()) {
                    continue;
                }
                if (it->second > max_similarity) {
                    max_similarity = it->second;
                    max_sim_exampler = c;
                }
            }
            // std::cout << "neighs ok" << std::endl;
            // assert(max_sim_exampler != -1);
            if (max_sim_exampler == -1) {
                //this point does not neighbors with any point caracterized as center
                no_center_neighbor++;
                max_sim_exampler = -1;
                max_similarity = -MAX_FLOAT;
                for (const auto& c : centers) {
                    float dist = std::fabs(nodes[n.id].x - nodes[c].x) + std::fabs(nodes[n.id].y - nodes[c].y);
                    if ((-dist) > max_similarity) {
                        max_similarity = -dist;
                        max_sim_exampler = c;
                    }
                }
            }
            assert(max_sim_exampler != -1);

            n.id_cluster = max_sim_exampler;
            // std::cout << "bla" << std::endl;
            clusters[max_sim_exampler].insert(n.id);
            // std::cout << "----" << std::endl;
        }

        std::cout << "clusters - ok2, #centers = " << centers.size() << " , #clusters = " << clusters.size() << std::endl;
        std::cout << "#points with no center neighbors found = " << no_center_neighbor << std::endl;

        int total_points_assigned = 0;
        int min_sz = MAX_INT;
        int max_sz = -MAX_INT;
        for (const auto& c : clusters) {
            total_points_assigned += c.second.size();
            min_sz = std::min(min_sz, (int)c.second.size());
            max_sz = std::max(max_sz, (int)c.second.size());
        }
        if (total_points_assigned != (int)nodes.size()) {
            std::cout << std::endl << "#nodes = " << nodes.size() << " , and #assigned points = " << total_points_assigned << std::endl;
            assert(false);
        }
        std::cout << "clusters - ok3" << std::endl;
        std::cout << "Min cluster size = " << min_sz << std::endl;
        std::cout << "Max cluster size = " << max_sz << std::endl;

        // std::cout << std::endl;
        // std::cout << std::endl;

        // for (const auto& n : nodes) {
        //     std::cout << "-------" << n.id << "-------" << std::endl;
        //     bool found = false;
        //     for (const auto& c : centers) {
        //         if (std::fabs(n.max_criterion - nodes[c].max_criterion) <= 1e-6) {
        //             found = true;
        //             break;
        //         }
        //     }
        //     if (!found) {
        //         std::cout << "Current max_criterion = " << n.max_criterion << std::endl;
        //         assert(found);
        //     }
        // }

        // std::cout << "Total number of centers = " << centers.size() << std::endl;
        // std::vector<std::set<int> > rtn_cl;
        // return rtn_cl;

        /*for (auto& n : nodes) {
            // std::cout << "---- " << n.id << " ----" << std::endl;
            float max_crit = -MAX_FLOAT;
            int max_neigh = -1;
            for (const auto& neigh : n.messages[Type::AVAILABILITY]) {
                // std::cout << neigh.first << " -> " << std::fixed << neigh.second << std::endl;
                if ((neigh.second + n.messages[Type::RESPONSIBILITY][neigh.first]) > max_crit) {
                    max_crit = neigh.second + n.messages[Type::RESPONSIBILITY][neigh.first];
                    max_neigh = neigh.first;
                }
                //max_crit = std::max(max_crit, neigh.second);
            }
            n.max_criterion = (int)(max_crit * 1e2);
            clusters[n.max_criterion].insert(n.id);

            // std::cout << "  Max found = " << n.max_criterion << " and the neigh is " << max_neigh << std::endl;
            if (max_neigh == n.id) {
                centers.push_back(n.id);
                // std::cout << " INSERTED as exampler" << std::endl;
            }
        }*/

        /*int assigned_pnts = 0;
        for (const auto& elem : clusters) {
            std::cout << "---- cluster " << elem.first << " ----" << std::endl;
            for (const auto& c : elem.second) {
                std::cout << c << std::endl;
            }
            assigned_pnts += elem.second.size();
            rtn_cl.push_back(elem.second);
        }
        if (assigned_pnts != pnts_sz) {
            std::cout << "Assigned points = " << assigned_pnts << " , points size = " << pnts_sz << std::endl;
            assert(false);
        }*/
    }

    void Affinity_Propagation::print_matrix(int node_id) {
        assert(node_id >= 0);
        assert(node_id < (int)nodes.size());

        std::cout << "--- Start printing SIMILAARITY matrix of " << node_id << " node ---" << std::endl;
        for (const auto& elem : nodes[node_id].messages[Type::SIMILARITY]) {
            std::cout << elem.first << " = " << elem.second << std::endl;
        }
        std::cout << "--- End of print matrix ---" << std::endl << std::endl;

        std::cout << "--- Start printing RESPONSIBILITY matrix of " << node_id << " node ---" << std::endl;
        for (const auto& elem : nodes[node_id].messages[Type::RESPONSIBILITY]) {
            std::cout << elem.first << " = " << elem.second << std::endl;
        }
        std::cout << "--- End of print matrix ---" << std::endl << std::endl;

        std::cout << "--- Start printing AVAILABILITY matrix of " << node_id << " node ---" << std::endl;
        for (const auto& elem : nodes[node_id].messages[Type::AVAILABILITY]) {
            std::cout << elem.first << " = " << elem.second << std::endl;
        }
        std::cout << "--- End of print matrix ---" << std::endl << std::endl;

        // std::cout << "--- Start printing CRITERION matrix of " << node_id << " node ---" << std::endl;
        // for (const auto& elem : nodes[node_id].messages[CRITERION]) {
        //     std::cout << elem.first << " = " << elem.second << std::endl;
        // }
        // std::cout << "--- End of print matrix ---" << std::endl << std::endl;
    }

    void Affinity_Propagation::print_clusters(bool show_lines /* = false */) {
        //writes a matlab file
        std::ofstream txt(data_name + "_affinity.m");
        txt << "clear; clc;" << std::endl;
        txt << "close all;" << std::endl << std::endl;

        txt << "colorspec = {'r'; 'g'; 'b'; 'c'; 'm'; [0.9290 0.6940 0.1250]; [0.4660 0.6740 0.1880]; [0.6350 0.0780 0.1840]};" << std::endl << std::endl;

        int cl_num = 0;
        for (const auto& cl : clusters) {
            if (cl.second.size() == 0) {
                continue;
            }
            txt << std::endl << "%---- " << cl_num << " ----" << std::endl;
            txt << "x" << cl_num << " = [ ";
            for (const auto& pnt_id : cl.second) {
                txt << nodes[pnt_id].x << " ";
            }
            txt << "];" << std::endl;

            txt << "y" << cl_num << " = [ ";
            for (const auto& pnt_id : cl.second) {
                txt << nodes[pnt_id].y << " ";
            }
            txt << "];" << std::endl;

            if (cl_num) {
                txt << "hold on;" << std::endl;
            }
            // txt << "plot(x" << cl_num << ", y" << cl_num << ", 'Color', colorspec{" << 1+(cl_num%8) << "}, 'LineStyle', '.');" << std::endl;
            txt << "plot(x" << cl_num << ", y" << cl_num << ", '.', 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl;

            txt << std::endl;
            txt << "hold on;" << std::endl;
            // txt << "plot(" << nodes[cl.first].x << ", " << nodes[cl.first].y << ", 'Color', colorspec{" << 1+(cl_num%8) << "}, 'LineStyle', 'x');" << std::endl << std::endl; 
            txt << "plot(" << nodes[cl.first].x << ", " << nodes[cl.first].y << ", 'x', 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl << std::endl; 
            if (show_lines) {
                for (auto m : cl.second) {
                    txt << "hold on" << std::endl;
                    txt << "plot( [" << nodes[cl.first].x << " " << nodes[m].x << "], [" << nodes[cl.first].y << " " << nodes[m].y << "], 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl;
                }
            }
            cl_num++;
        }

        // txt << std::endl;
        // for (const auto& c : centers) {
        //     txt << "hold on;" << std::endl;
        //     txt << "plot(" << nodes[c].x << ", " << nodes[c].y << ", 'LineStyle', 'x');" << std::endl;
        // }
        txt << "title('Affinity Propagation')" << std::endl;
        txt.close();
    }

    void Affinity_Propagation::print_clusters_v2() {
        //writes a matlab file
        std::ofstream txt(data_name + "_affinity_cluster_visualization_v2.m");
        txt << "clear; clc;" << std::endl;
        txt << "close all;" << std::endl << std::endl;

        txt << "colorspec = {'r'; 'g'; 'b'; 'c'; 'm'; [0.9290 0.6940 0.1250]; [0.4660 0.6740 0.1880]; [0.6350 0.0780 0.1840]};" << std::endl << std::endl;
        txt << std::endl;

        txt << "x = [ ";
        for (const auto& n : nodes) {
            txt << n.x << " "; 
        } 
        txt << "];" << std::endl;

        txt << "y = [ ";
        for (const auto& n : nodes) {
            txt << n.y << " "; 
        } 
        txt << "];" << std::endl;

        txt << "z = [ ";
        for (const auto& n : nodes) {
            txt << n.id_cluster << " "; 
        } 
        txt << "];" << std::endl;

        // txt << "cl_ids = [ ";
        // for (const auto& c : centers) {
        //     txt << c << " ";
        // }
        // txt << "];" << std::endl;

        txt << std::endl;
        // txt << "nn = size(x, 2);" << std::endl;
        // txt << "for cl = 1 : size(cl_ids, 2)" << std::endl;
        // txt << "   cl_id = cl_ids(cl);" << std::endl;
        // txt << "   for i = 1 : nn" << std::endl;
        // txt << "      if ( z(i) == cl_id )" << std::endl;
        // txt << "         hold on;" << std::endl;
        // txt << "         plot(x(i), y(i), 'Color', colorspec{1 + mod(cl_id, 8)}, 'LineStyle', '.');" << std::endl;
        // txt << "      end" << std::endl;
        // txt << "   end" << std::endl;
        // txt << "end" << std::endl;
        txt << "scatter(x, y, 150, z, '.');" << std::endl;

        txt.close();
    }


    void Affinity_Propagation::test() {
        //similarity(0, 0) = {{-22, -7, -6, -12, -17}, {-7, -22, -17, -17, -22}, {-6, -17, -22, -18, -21}, {-12, -17, -18, -22, -3}, {-17, -22, -21, -3, -22}};
        nodes.clear();
        clusters.clear();
        pnts_sz = 0;

        max_iters = 1;
        lambda = 0;


        for (int i = 0; i < 5; i++) {
            Node node;
            switch (i) {
                case 0 :
                    node.name = "Alice";
                    node.messages[Type::SIMILARITY][0] = -22;
                    node.messages[Type::SIMILARITY][1] = -7;
                    node.messages[Type::SIMILARITY][2] = -6;
                    node.messages[Type::SIMILARITY][3] = -12;
                    node.messages[Type::SIMILARITY][4] = -17;
                    break;
                case 1 :
                    node.name = "Bod";
                    node.messages[Type::SIMILARITY][0] = -7;
                    node.messages[Type::SIMILARITY][1] = -22;
                    node.messages[Type::SIMILARITY][2] = -17;
                    node.messages[Type::SIMILARITY][3] = -17;
                    node.messages[Type::SIMILARITY][4] = -22;
                    break;
                case 2 :
                    node.name = "Cary";
                    node.messages[Type::SIMILARITY][0] = -6;
                    node.messages[Type::SIMILARITY][1] = -17;
                    node.messages[Type::SIMILARITY][2] = -22;
                    node.messages[Type::SIMILARITY][3] = -18;
                    node.messages[Type::SIMILARITY][4] = -21;
                    break;
                case 3 :
                    node.name = "Doug";
                    node.messages[Type::SIMILARITY][0] = -12;
                    node.messages[Type::SIMILARITY][1] = -17;
                    node.messages[Type::SIMILARITY][2] = -18;
                    node.messages[Type::SIMILARITY][3] = -22;
                    node.messages[Type::SIMILARITY][4] = -3;
                    break;
                case 4 :
                    node.name = "Edna";
                    node.messages[Type::SIMILARITY][0] = -17;
                    node.messages[Type::SIMILARITY][1] = -22;
                    node.messages[Type::SIMILARITY][2] = -21;
                    node.messages[Type::SIMILARITY][3] = -3;
                    node.messages[Type::SIMILARITY][4] = -22;
                    break;
                default :
                    node.name = "INVALID";
            }
            node.neighs.insert(0);
            node.neighs.insert(1);
            node.neighs.insert(2);
            node.neighs.insert(3);
            node.neighs.insert(4);

            node.id = nodes.size();
            nodes.push_back(node);
        }
        pnts_sz = nodes.size();

        run();
        for (const auto& n : nodes) {
            std::cout << std::endl << std::endl;
            print_matrix(n.id);
        }

        extract_clusters();

        for (const auto& cl : clusters) {
            std::cout << "--- cluster ---" << std::endl;
            for (const auto& id : cl.second) {
                std::cout << id << std::endl;
            }
        }
    }
