#ifndef DATATYPES_EXTRA_CSR
#define DATATYPES_EXTRA_CSR

#include "datatypes_spec.h"

template<int DIM>
struct CSR_point {
  CID_TYPE id_cluster;  
  COORD_TYPE coord[DIM-1];
};

struct CSR_pos {
  COORD_TYPE pointer;
  ac_int<PID_BITS, false> index;
};

typedef CSR_point<DATA_DIM> CSR_CRD_T;

typedef CSR_pos CSR_POS_T;

#endif
