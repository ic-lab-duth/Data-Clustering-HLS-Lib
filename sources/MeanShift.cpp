#include <fstream>
#include "MeanShift.h"

// Add a new point to points vector
void MeanShift::add_point(float x, float y) {
  Point pnt;
  pnt.id = points.size();
  pnt.x = x;
  pnt.y = y;

  points.push_back(pnt);
}

// Add points be reading data file
void MeanShift::add_data(std::vector<std::pair<float, float> > data) {
  for (auto pnt : data) {
    add_point(pnt.first, pnt.second);
  }
}

// Initialize clusters by assinging a cluster center to each point
void MeanShift::initialize_clusters() {

  Cluster new_cluster;

  for (auto pnt : points) {
    new_cluster.x = pnt.x;
    new_cluster.y = pnt.y;

    clusters.push_back(new_cluster);
  }
}

// 2nd power of Euclidean distance between 2 clusters
float MeanShift::compute_distance(Cluster c1, Cluster c2) {
  float distance;
  
  if (dist_calculator == EUCLIDEAN) {
    distance = pow((c1.x - c2.x),2) + pow((c1.y - c2.y),2);
    distance = sqrt(distance);
  } else if (dist_calculator == MANHATTAN) {
    distance = abs(c1.x - c2.x) + abs(c1.y - c2.y);
  } else { // dist_calculator == MAX
    distance = (abs(c1.x - c2.x) > abs(c1.y - c2.y)) ? abs(c1.x - c2.x) : abs(c1.y - c2.y);
  } 

  return distance;
}

// 2nd power of Euclidean distance between a point and a cluster
float MeanShift::compute_distance(Point pnt, Cluster cls) {
  float distance;

  if (dist_calculator == EUCLIDEAN) {
    distance = pow((pnt.x - cls.x),2) + pow((pnt.y - cls.y),2);
    distance = sqrt(distance);
  } else if (dist_calculator == MANHATTAN) {
    distance = abs(pnt.x - cls.x) + abs(pnt.y - cls.y);
  } else { // dist_calculator == MAX
    distance = (abs(pnt.x - cls.x) > abs(pnt.y - cls.y)) ? abs(pnt.x - cls.x) : abs(pnt.y - cls.y);
  } 

  return distance;
}

// Returns the weight of a point depending on the GAUSSIAN kernel function
float MeanShift::Gaussian_Kernel(float distance, float radius) {

  float t = (distance * distance) / (radius * radius);

  // return (1/(2*M_PI))*exp(-t/2);
  return exp(-t/2);
}

// Returns the weight of a point depending on the EPANECHNIKOV kernel function
float MeanShift::Epanechnikov_Kernel(float distance, float radius){

  float t = (distance * distance) / (radius * radius);

  return (t < 1) ? 1-t : 0;
}

// Find the new cluster centers
std::vector<Cluster> MeanShift::compute_weighted_new_clusters() {

  std::vector<Cluster> new_cluster_centers;
  Cluster tmp_center;
  float K, distance, denominator;

  for (auto cls : clusters) {
    tmp_center.x = 0.0;
    tmp_center.y = 0.0;

    denominator = 0.0;

    for (auto pnt : points) {

      distance = compute_distance(pnt, cls);

      if (distance < radius) {
            
        if (kernel == GAUSSIAN) {
          K = Gaussian_Kernel(distance, radius);
        } else if (kernel == EPANECHNIKOV) {
          K = Epanechnikov_Kernel(distance, radius);
        } else { // kernel == NONE
          K = 1;
        }

        tmp_center.x += pnt.x * K;
        tmp_center.y += pnt.y * K; 
        
        denominator += K;
      }
    }
    
    tmp_center.x = tmp_center.x / denominator;
    tmp_center.y = tmp_center.y / denominator;

    bool found = false;
    for (auto c :new_cluster_centers) {
      if ( compute_distance(c, tmp_center) < radius*0.05 ) {
        found = true;
        break;
      }
    }
    if (!found) {
      new_cluster_centers.push_back(tmp_center);
    }
  }

  return new_cluster_centers;

}

// Checks if the change in the cluster centers is small enough to finish optimization
bool MeanShift::check_similarity(std::vector<Cluster> new_clusters) {

  bool similar = true;
  float dist, max_dist;
  max_dist = 0;

  if (clusters.size() != new_clusters.size()) {
    similar = false;
  } else {
    std::vector<Cluster>::iterator c_i = clusters.begin();
    std::vector<Cluster>::iterator nc_i = new_clusters.begin();
    while (c_i != clusters.end()) {
      dist = compute_distance(*c_i, *nc_i);
      if (dist > max_dist) {
        max_dist = dist;
      }
      c_i++;
      nc_i++;
    }

    if (max_dist > tolerance) {
      similar = false;
    }
  }
  
  return similar;
}

// Updates clusters vector with the new cluster centers
void MeanShift::update_clusters(std::vector<Cluster> new_clusters) {  
  clusters.clear();

  for (auto cls: new_clusters) {

    cls.id = clusters.size();
    clusters.push_back(cls);
  }
}

// Connects each point to its corresponind cluster
void MeanShift::attach_points_to_clusters() {

  std::vector<Cluster>::iterator min_iter;
  float min_dist;
  float cur_dist;


  for (auto pnt : points) {
    min_dist = MAXFLOAT;
    if (clusters.size() == 0) {
      std::cerr << "ERROR: NO CLUSTERS FOUND! Exiting..." << std::endl;
      exit(1);
    }
    for (std::vector<Cluster>::iterator i=clusters.begin();i!=clusters.end(); ++i) {
      Cluster cls = *i;
      cur_dist = compute_distance(pnt, cls);
      if (cur_dist < min_dist) {
        min_dist = cur_dist;
        min_iter = i;
      }
    }

    min_iter->points.insert(pnt.id);
  }  
  
}

// Run Mean Shift clustering
void MeanShift::run() {

  std::vector<Cluster> new_clusters;

  initialize_clusters();

  for (int i=0; i<maxIterations; i++) {

    new_clusters = compute_weighted_new_clusters();

    if (check_similarity(new_clusters)) {
      std::cout << "Finished after " << i << " iterations." << std::endl;
      break;
    }
    
    update_clusters(new_clusters);
  }

  attach_points_to_clusters();
}

// Export clusters into MATLAB plot format
void MeanShift::print_clusters() {

  std::ofstream txt(data_name + "_mean_shift.m");
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
  txt << "title('Mean Shift')" << std::endl;
  txt.close();
}

// Display clusters results on terminal window
void MeanShift::display_clusters() {

  for (auto cls : clusters) {
    std::cout << "X: " << cls.x << " Y: " << cls.y << " --> Points in cluster:" << cls.points.size() << std::endl;
  }

  std::cout << "Number of Clusters: " << clusters.size() << std::endl;
}


// DEPRECATED CODE

// std::vector<Cluster> MeanShift::compute_weighted_new_clusters() {

//   std::vector<Cluster> new_cluster_centers;

//   Cluster tmp_center;

//   float K;
//   float distance, denominator;

//   for (std::vector<Cluster>::iterator cls = clusters.begin(); cls != clusters.end(); ++cls) {
//     tmp_center.x = 0.0;
//     tmp_center.y = 0.0;

//     denominator = 0.0;

//     for (auto pnt : points) {


//       distance = compute_distance(pnt, *cls);

//       if (distance <= radius) {

//         if (kernel == GAUSSIAN) {
//           K = Gaussian_Kernel(distance, radius);
//         } else if (kernel == EPANECHNIKOV) {
//           K = Epanechnikov_Kernel(distance, radius);
//         } else {
//           std::cerr << "WRONG KERNEL. Exiting..." << std::endl;
//           exit(1);
//         }

//         tmp_center.x += pnt.x * K;
//         tmp_center.y += pnt.y * K; 
      
//         denominator += K;
//       }
//     }
//     if (denominator == 0) {
//       std::cout << "0" <<std::endl;
//     } else {
//       tmp_center.x = tmp_center.x / denominator;
//       tmp_center.y = tmp_center.y / denominator;

//       cls->x = tmp_center.x;
//       cls->y = tmp_center.y;
//     }
//   }

//   for (auto cls : clusters) {
//     bool found = false;
//     for (auto c : new_cluster_centers) {
//       if ( pow(c.x - cls.x,2) + pow(c.y - cls.y,2) < pow(radius*0.05,2)) {
//         found = true;
//         break;
//       }
//     } 
//     if (!found) {
//       new_cluster_centers.push_back(cls);
//     }
//   }

//   return new_cluster_centers;

// }