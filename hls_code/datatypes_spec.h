#ifndef DATATYPES_SPEC_H
#define DATATYPES_SPEC_H

#include <ac_int.h>
#include <ac_fixed.h>
#include <ac_math.h>
#include <array>

/*
*   PARAMETERS
*/
const int M_ITERAT = 100;   // define max run iterations

const int DATA_DIM = 2;     // define dimensionality of data (x,y,z,...)
const int N_POINTS = 50;  // define amount of points
const int K_CLSTRS = 3;     // define amount of clusters

const int MAX_VALUE = 390892;
const int MIN_VALUE = 139779;
const int GRANULARITY = 1;

/*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
*
* END OF PARAMETER DECLARATION 
* (do not make changeσ το the rest of the document)
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/


/*
*   HELPER FUNCTIONS FOR DATATYPES INITIALIZATION
*/
template<const int MAX_V, const int MIN_V, const int G>
struct COORDINATES_DATATYPE_INITIALIZATION {
  static const bool sign = (MIN_V < 0) ? true : false ;
  static const int i_bits = ac::nbits<MAX_V>::val;
  static const int bits = (sign) ? i_bits + G + 1 : i_bits + G;
  
};

template<const int MAX_V, const int MIN_V, const int N, const int G>
struct DISTANCE_DATATYPE_INITIALIZATION {
  static const bool sign = false;
  static const int i_bits = 2 * ac::nbits<MAX_V-MIN_V>::val + ac::nbits<N>::val;
  static const int bits = (sign) ? i_bits + G + 1 : i_bits + G;
  
};

template<const int MAX_V, const int N, const int K, const int G>
struct ACCUMULATOR_DATATYPE_INITIALIZATION {
  static const bool sign = false;
  static const int i_bits = ac::nbits<MAX_V>::val + ac::nbits<2*N/K>::val;
  static const int bits = (sign) ? i_bits + G + 1 : i_bits + G;
};

template<const int X>
struct CALCULATE_LOG2 {
  static const int bits = ac::nbits<X>::val;
};


/*
*   AUTO CALCULATED VALUES
*/
static const int PID_BITS = CALCULATE_LOG2<N_POINTS>::bits;  // define amount of bits needed to save point ids
static const int CID_BITS = CALCULATE_LOG2<K_CLSTRS>::bits;  // define amount of bits needed to save clusters ids

static const int COORD_BITS       = COORDINATES_DATATYPE_INITIALIZATION<MAX_VALUE, MIN_VALUE, GRANULARITY>::bits;
static const int COORD_I_BITS     = COORDINATES_DATATYPE_INITIALIZATION<MAX_VALUE, MIN_VALUE, GRANULARITY>::i_bits;
static const int COORD_SIGNEDNESS = COORDINATES_DATATYPE_INITIALIZATION<MAX_VALUE, MIN_VALUE, GRANULARITY>::sign;

static const int DIST_BITS       = DISTANCE_DATATYPE_INITIALIZATION<MAX_VALUE, MIN_VALUE, N_POINTS, GRANULARITY>::bits;
static const int DIST_I_BITS     = DISTANCE_DATATYPE_INITIALIZATION<MAX_VALUE, MIN_VALUE, N_POINTS, GRANULARITY>::i_bits;
static const int DIST_SIGNEDNESS = DISTANCE_DATATYPE_INITIALIZATION<MAX_VALUE, MIN_VALUE, N_POINTS, GRANULARITY>::sign;

static const int ACC_BITS       = ACCUMULATOR_DATATYPE_INITIALIZATION<MAX_VALUE, N_POINTS, K_CLSTRS, GRANULARITY>::bits;
static const int ACC_I_BITS     = ACCUMULATOR_DATATYPE_INITIALIZATION<MAX_VALUE, N_POINTS, K_CLSTRS, GRANULARITY>::i_bits;
static const int ACC_SIGNEDNESS = ACCUMULATOR_DATATYPE_INITIALIZATION<MAX_VALUE, N_POINTS, K_CLSTRS, GRANULARITY>::sign;


/*
*   MAIN DATATYPES DEFINITION
*/
typedef ac_int<CID_BITS, false> CID_TYPE;       // Define size of data type for cluster IDs

// typedef ac_int<PID_BITS, false> COUNTER_TYPE;   // datatype for the counter variables (used to count the amount of points in each cluster
typedef ac_fixed<PID_BITS, PID_BITS, false, AC_TRN, AC_SAT> COUNTER_TYPE;   // datatype for the counter variables (used to count the amount of points in each cluster)

typedef ac_fixed<COORD_BITS, COORD_I_BITS, COORD_SIGNEDNESS, AC_TRN, AC_SAT> COORD_TYPE; // datatype for the points' coordinates variables
typedef ac_fixed<DIST_BITS, DIST_I_BITS, DIST_SIGNEDNESS, AC_TRN, AC_SAT>    DIST_TYPE;  // datatype for distance and cost variables
typedef ac_fixed<ACC_BITS, ACC_I_BITS, ACC_SIGNEDNESS, AC_TRN, AC_SAT>       ACCU_TYPE;  // datatype for the accumulators variables (used to update C centers)


template<int DIM>
struct point {
  CID_TYPE id_cluster;  // id of the cluster that this point belongs to

  COORD_TYPE coord[DIM];

};

typedef point<DATA_DIM> P_TYPE;
typedef std::array<COORD_TYPE, DATA_DIM> C_TYPE;

#endif
