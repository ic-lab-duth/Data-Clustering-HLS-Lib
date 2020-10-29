#ifndef DATATYPES_SPEC_H
#define DATATYPES_SPEC_H

#include <set>
#include <ac_int.h>
#include <ac_fixed.h>

#include <cmath>

/*
*   PARAMETERS
*/
#define M_ITERAT 100   // define max run iterations

#define DATA_DIM 2     // define dimensionality of data (x,y,z,...)
#define N_POINTS 6000  // define amount of points
#define K_CLSTRS 3     // define amount of clusters

#define PID_SIZE 13  // define amount of bits needed to save point ids
#define CID_SIZE 2  // define amount of bits needed to save clusters ids

/*
*   DATA TYPES
*/
typedef ac_int<PID_SIZE,false> PNTID_TYPE; // Define size of data type for point IDs
typedef ac_int<CID_SIZE,false> CLTID_TYPE; // Define size of data type for cluster IDs

typedef ac_fixed<21,20,true,AC_TRN,AC_SAT> COORD_TYPE; // define type of the point's coordinates values

typedef ac_fixed<35,34,true,AC_TRN,AC_SAT> DIST_TYPE;  // data type for distance variables
typedef ac_fixed<45,45,false,AC_TRN,AC_SAT> COST_TYPE;  // data type for cost variables

// data types for clusters' centers update
// CNT_ACC_TYPE : Acummulate points coordinates
// COUNTER_TYPE : Amount of points belonging to cluster
typedef ac_fixed<33,32,false,AC_TRN,AC_SAT>  CNT_ACC_TYPE;
typedef ac_int<13,false> COUNTER_TYPE; 


template<int DIM>
struct point {
  PNTID_TYPE id;          // id of the point
  CLTID_TYPE id_cluster;  // id of the cluster that this point belongs to

  COORD_TYPE x;
  COORD_TYPE y;
};

template<int DIM>
struct cluster {
  CLTID_TYPE id;

  COORD_TYPE x;
  COORD_TYPE y;

  std::set<PNTID_TYPE> points;
};

template<int DIM>
struct center {
  COORD_TYPE x;
  COORD_TYPE y;
};

typedef point<DATA_DIM> P_TYPE;
typedef cluster<DATA_DIM> C_TYPE;
typedef center<DATA_DIM> CENTER_T;



#endif