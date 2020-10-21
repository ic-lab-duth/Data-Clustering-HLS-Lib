#ifndef DBSCAN_H
#define DBSCAN_H

#include <vector>
#include <cmath>
#include <set>
#include <string>
#include <iostream>
#include <fstream>
#include <assert.h>

//dionisis
#include <limits>

#define UNCLASSIFIED -1
#define CORE_POINT 1
#define BORDER_POINT 2
#define NOISE -2
#define SUCCESS 0
#define FAILURE -3

namespace DensityBased {

typedef struct Point_
{
    int id;
    float x; 
    float y;
    int id_cluster;  // clustered ID
}Point;

struct Cluster {
    int id;
    std::set<int> points;
};

class DBScan {
public:    
    DBScan(unsigned int minPts, float eps){
        m_minPoints = minPts;
        m_epsilon = eps;
    }
    ~DBScan(){}

    void add_point(float x, float y);
    void add_data(std::vector<std::pair<float, float> > data);
    void run();
    std::vector<int> calculateCluster(Point point);
    int expandCluster(Point point, int id_cluster);
    inline double calculateDistance(Point pointCore, Point pointTarget);
    void group_points_to_clusters();
    void print_clusters();

    void set_data_name(std::string value) {
        data_name = value;
    }
    int getTotalPointSize() {
        return m_pointSize;
    }
    int getMinimumClusterSize() {
        return m_minPoints;
    }
    int getEpsilonSize() {
        return m_epsilon;
    }

    std::vector<Point> m_points;
    std::vector<Cluster> clusters;
    unsigned int m_pointSize;
    unsigned int m_minPoints;
    float m_epsilon;
    std::string data_name;
    float max_coord = - std::numeric_limits<int>::max();
};
}

#endif // DBSCAN_H