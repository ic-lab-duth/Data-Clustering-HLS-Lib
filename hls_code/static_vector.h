#ifndef STATIC_VECTOR_H
#define STATIC_VECTOR_H

#include <ac_int.h>
#include <ac_fixed.h>
#include <ac_math.h>
#include "datatypes_spec.h"
#include <mc_scverify.h>


template<int DIM, int N>
class uniqueStack {
private:
  C_TYPE *mem;
  PID_TYPE pnt;

  DIST_TYPE distance(C_TYPE t1, C_TYPE t2) {
    DIST_TYPE dist = 0;
    for (int j=0; j<DIM; j++) {
      dist += (t1[j]-t2[j])*(t1[j]-t2[j]);
    }
    ac_math::ac_sqrt(dist, dist);

    return dist;
  }
 
public:
  uniqueStack(): pnt(0) {};
  uniqueStack(C_TYPE *m): pnt(0), mem(m) {};
  ~uniqueStack() {};

  void push_update(C_TYPE val, DIST_TYPE tol) {
    bool update = true;
    for (int i=0; i<N; i++) {
      if (i < pnt) {
        if (distance(val, mem[i]) < tol) {
          update = false;
        }
      } 
    }

    if (update) push(val);

  };

  PID_TYPE size() {
    return pnt;
  }

  void update_current(PID_TYPE ind, C_TYPE cent) {
    mem[ind] = cent;
  }

  void reset_pointer() {
    pnt = 0;
  }

  void push(C_TYPE val) {
    mem[pnt++] = val;
  };

  C_TYPE at(PID_TYPE indx) {
    return mem[indx];
  }
  
};

#pragma hls_design interface
template<int DIM, int N>
class uniqueQueue {
private:
  PID_TYPE *mem;
  PID_TYPE pnt;


public:
  uniqueQueue(): pnt(0) {};
  uniqueQueue(PID_TYPE *m): pnt(0), mem(m) {};
  ~uniqueQueue() {};

  PID_TYPE size() {
    return pnt;
  }

  void reset_queue() {
    pnt = 0;
  }
  
  void resize_Q(PID_TYPE x) {
    pnt = x;
  }

  void push_unique(PID_TYPE val) {
    bool isUnique = true;
    for (int i=0; i<N; i++) {
      if (i < pnt) {
        if (mem[i] == val) {
          isUnique = false;
        }
      } 
    }
    
    if (isUnique)
      push(val);
  };

  void push(PID_TYPE val) {
    mem[pnt++] = val;
  }

  PID_TYPE at(PID_TYPE indx) {
    return mem[indx];
  }
  
};

#endif
