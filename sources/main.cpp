#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <map>
#include <string.h>
#include <assert.h>
#include <string>
#include <algorithm>

#include "KMeans_basic.h"
#include "KMeans_soft.h"
#include "AffinityPropagation.h"
#include "DBScan.h"

#include "MeanShift.h"

bool read_line_as_tokens(std::istream &is, std::vector<std::string> &tokens){
    tokens.clear();

    std::string line;
    getline(is, line);
    while (is && tokens.empty()){
        std::string token = "";
        for (unsigned i = 0; i < line.size(); ++i){
             char currChar = line[i];
            if (currChar==' '){
                if (!token.empty()){
                    // Add the current token to the list of tokens
                    tokens.push_back(token);
                    token.clear();
                }
                // else if the token is empty, simply skip the whitespace or special char
            }else{
                // Add the char to the current token
                token.push_back(currChar);
            }
        }

        if (!token.empty()){
            tokens.push_back(token);
        }

        if (tokens.empty())
            // Previous line read was empty. Read the next one.
            std::getline(is, line);
    }

    return !tokens.empty();
}

bool read_file(std::string file, std::vector<std::pair<float, float> >& data) {
    std::ifstream dot_ops((file).c_str());
    if(!dot_ops.good()){
        std::cout << "cannot read file : " << file << std::endl;
        return false;
    }

    data.clear();

    std::vector<std::string> tokens;
    bool valid = read_line_as_tokens(dot_ops, tokens);
    int line=1;
    
    bool found = false;
    std::string pin_name;
    while (valid){
        if (tokens.size() != 0) {
            if (tokens.size() != 2) {
                std::cout << "Error in line " << line << ": Tokens are ";
                for (auto token : tokens) {
                    std::cout << "'" << token << "' ";
                }
                std::cout << std::endl;
                assert(false);
            }
            
            data.push_back(std::make_pair(std::stof(tokens[0]), std::stof(tokens[1])));
        }

        valid = read_line_as_tokens(dot_ops, tokens);
        line++;
    }
    dot_ops.close();
    return found;
}

int main(int argc, char *argv[]) {
    std::string filename = "";
    std::string method = "";
    
    bool found_file = false;
    bool found_method = false;
    for (int i=0; i<argc; i++) {
        std::cout << argv[i] << " ";

        if (!strcmp(argv[i], "-file")) {
            assert(i+1 < argc);
            filename = (std::string)argv[i+1];
            found_file = true;
        }

        if (!strcmp(argv[i], "-method")) {
            assert(i+1 < argc);
            method = (std::string)argv[i+1];
            std::transform(method.begin(), method.end(), method.begin(), ::toupper);
            found_method = true;
        }
    }
    std::cout << std::endl;
    assert(found_file);
    assert(found_method);

    //filename = "./clustering_data/data_1.txt";

    std::vector<std::pair<float, float>> data;
    read_file(filename, data);
    std::cout << "Read " << data.size() << " data from file..." << std::endl;

    size_t pos = 0;
    std::string token;
    while ((pos = filename.find("/")) != std::string::npos) {
        token = filename.substr(0, pos);
        // std::cout << token << std::endl;
        filename.erase(0, pos + 1);
    }
    if ((pos = filename.find(".")) != std::string::npos) {
        filename = filename.substr(0, pos);
    }


    if (method == "KMEANS_BASIC") {
        std::cout << "Please enter the desired number of K = " << std::endl;
        int K_value;
        std::cin >> K_value;
        std::cout << std::endl;
        KMeans_basic kmeans(100, K_value);

        kmeans.set_data_name(filename);
        kmeans.add_data(data);
        kmeans.initialize_clusters();
        kmeans.run();
        kmeans.print_clusters();
    }  else if (method == "KMEANS_SOFT") {
        std::cout << "Please enter the desired number of K = " << std::endl;
        int K_value;
        std::cin >> K_value;
        std::cout << std::endl;
        soft::KMeans_soft kmeans(100, K_value);

        kmeans.set_data_name(filename);
        kmeans.add_data(data);
        kmeans.initialize_clusters();
        kmeans.run();
        kmeans.print_clusters();
    } else if (method == "AFFINITY") {
        // -------- AFFINITY CLUSTERING --------- //
        Affinity_Propagation cluster;
        cluster.set_data_name(filename);

        cluster.add_data(data);
        cluster.prepare();
        cluster.set_max_iters(10);
        cluster.set_lambda(0.5);
        cluster.run();
        cluster.extract_clusters();
        std::cout << "Total clusters found = " << cluster.clusters.size() << std::endl;
        cluster.print_clusters(true);
    } else if (method == "DBSCAN") {
        DensityBased::DBScan dbscan(5, 0.0005);
        dbscan.set_data_name(filename);

        dbscan.add_data(data);
        dbscan.run();
        std::cout << "Total clusters found = " << dbscan.clusters.size() << std::endl;
        dbscan.print_clusters();
    } else if (method == "MEAN_SHIFT") {
        MeanShift mshift;
        mshift.set_data_name(filename);

        mshift.add_data(data);
        mshift.set_max_iterations(100);
        mshift.set_radius(3700);
        mshift.set_tolerance(0.00001);
        mshift.set_kernel("gaussian");
        mshift.set_dist_method("euclidea");

        mshift.run();
        mshift.display_clusters();
        mshift.print_clusters();
    } else {
        std::cout << "Unknown clustering method." << std::endl;
    }

    return 0;
}