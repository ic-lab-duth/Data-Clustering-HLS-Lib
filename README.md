# Clustering Algorithms using High Level Synthesis



## Differences between C++ and HLS implementations

* C++ version 

  ```
  1. assign points to the current clusters
  2. compute new centers and update clusters
  3. calculate cost
  ```

* HLS version 

  ```
  1. assign points to the current clusters
  2. calculate cost
  3. compute new centers and update clusters
  ```

In K-Means, the cost represents the sum of all points from their centers ($c_j$ represents the center where point *i* has been assigned to).


$$
cost = Σ_{i=1}^Ndist(p_i-c_j)
$$


In C++ version the cost is calculated by summarizing the distances of each point to each cluster center, although the clusters have their center been updated. This means that the points may not belong to that cluster anymore.

In HLS version the cost is calculated before the update of the centers. This means that there is no possibility for each point to belong to another center in that point of time.

We can observe this difference due to our C++ implementation. Maybe with a different approach on our C++ implementation, this difference would never exist.

## Folder Organization

**Sources**: C++ implementation of K-Means, Mean Shift, DBScan and Affinity Propagation.

**hls_code**: HLS ready C++ code for each algorithm

**testbenches:** C++ and HLS testbech for each algorithm

**datasets:** Collection of self generated datasets.

**sparse_dataset_generator:** Generator of sparse datasets. 

