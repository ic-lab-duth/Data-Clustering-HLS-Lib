#include "DBScan.h"


#include <iostream>

namespace DensityBased {
void DBScan::add_data(std::vector<std::pair<float, float> > data) {
    for (auto pnt : data) {
        add_point(pnt.first, pnt.second);
    }
    m_pointSize = m_points.size();

    //normalize
    for (auto& p : m_points) {
        p.x = p.x / max_coord;
        p.y = p.y / max_coord;
    }
}

void DBScan::add_point(float x, float y) {
    Point p;

    p.id_cluster = -1;
    p.x = x;
    p.y = y;
    p.id = (int)m_points.size();

    m_points.push_back(p);

    max_coord = std::max(max_coord, x);
    max_coord = std::max(max_coord, y);
}

void DBScan:: run() {
    int tmp = 0;
    int id_cluster = 1;
    std::vector<Point>::iterator iter;
    for(iter = m_points.begin(); iter != m_points.end(); ++iter) {
        tmp ++;
        if ( iter->id_cluster == UNCLASSIFIED ) {
            if ( expandCluster(*iter, id_cluster) != FAILURE ) {
                id_cluster += 1;
            }
        }
        
        if (id_cluster == 3) {
            group_points_to_clusters();
            print_clusters();
            break;
        }
        
    }
    // group_points_to_clusters();
}

int DBScan::expandCluster(Point point, int id_cluster) {    
    std::vector<int> clusterSeeds = calculateCluster(point);

    if ( clusterSeeds.size() < m_minPoints ) {
        point.id_cluster = NOISE;
        return FAILURE;
    } else {
        int index = 0, indexCorePoint = 0;
        std::vector<int>::iterator iterSeeds;
        for( iterSeeds = clusterSeeds.begin(); iterSeeds != clusterSeeds.end(); ++iterSeeds) {
            m_points.at(*iterSeeds).id_cluster = id_cluster;
            if (m_points.at(*iterSeeds).x == point.x && m_points.at(*iterSeeds).y == point.y ) {
                indexCorePoint = index;
            }
            ++index;
        }
        clusterSeeds.erase(clusterSeeds.begin()+indexCorePoint);

        for( std::vector<int>::size_type i = 0, n = clusterSeeds.size(); i < n; ++i ) {
            std::vector<int> clusterNeighors = calculateCluster(m_points.at(clusterSeeds[i]));

            if ( clusterNeighors.size() >= m_minPoints ) {
                std::vector<int>::iterator iterNeighors;
                for ( iterNeighors = clusterNeighors.begin(); iterNeighors != clusterNeighors.end(); ++iterNeighors ) {
                    if ( m_points.at(*iterNeighors).id_cluster == UNCLASSIFIED || m_points.at(*iterNeighors).id_cluster == NOISE ) {
                        if ( m_points.at(*iterNeighors).id_cluster == UNCLASSIFIED ) {
                            clusterSeeds.push_back(*iterNeighors);
                            n = clusterSeeds.size();
                        }
                        m_points.at(*iterNeighors).id_cluster = id_cluster;
                    }
                }
            }
        }

        return SUCCESS;
    }
}

std::vector<int> DBScan::calculateCluster(Point point) {
    int index = 0;
    std::vector<Point>::iterator iter;
    std::vector<int> clusterIndex;
    for( iter = m_points.begin(); iter != m_points.end(); ++iter) {
        if ( calculateDistance(point, *iter) <= m_epsilon ) {
            clusterIndex.push_back(index);
        }
        index++;
    }
    return clusterIndex;
}

inline double DBScan::calculateDistance( Point pointCore, Point pointTarget ) {
    return std::sqrt(pow(pointCore.x - pointTarget.x,2) + pow(pointCore.y - pointTarget.y,2));
}

void DBScan::group_points_to_clusters() {
    std::set<int> cluster_ids;
    for (const auto& p : m_points) {
        cluster_ids.insert(p.id_cluster);
    }

    int cl_sz = cluster_ids.size();
    int total_pnts = 0;
    for (const auto& cl_id : cluster_ids) {
        Cluster new_cluster;
        new_cluster.id = cl_id;
        for (const auto& p : m_points) {
            if (p.id_cluster != cl_id) {
                continue;
            }
            new_cluster.points.insert(p.id);
        }
        clusters.push_back(new_cluster);
        total_pnts += new_cluster.points.size();
    }

    assert(cl_sz == (int)clusters.size());
    assert(total_pnts == (int)m_points.size());
}

void DBScan::print_clusters() { 
    //writes a matlab file
    std::ofstream txt(data_name + "_dbscan.m");
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
            txt << m_points[pnt_id].x * max_coord << " ";
        }
        txt << "];" << std::endl;

        txt << "y" << cl_num << " = [ ";
        for (const auto& pnt_id : cl.points) {
            txt << m_points[pnt_id].y * max_coord << " ";
        }
        txt << "];" << std::endl;

        if (cl_num) {
            txt << "hold on;" << std::endl;
        }
        //txt << "plot(x" << cl_num << ", y" << cl_num << ", 'Color', colorspec{" << 1+(cl_num%8) << "}, 'LineStyle', '.');" << std::endl;
        txt << "plot(x" << cl_num << ", y" << cl_num << ", '.', 'Color', colorspec{" << 1+(cl_num%8) << "});" << std::endl;

        txt << std::endl;
        cl_num++;
    }
    txt << "title('DBSCAN')" << std::endl;
    txt.close();
}

}