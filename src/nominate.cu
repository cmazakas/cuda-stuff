#include "../include/lib/nominate.hpp"

__global__
void nominate(
  int const assoc_size,
  int const* __restrict__ ta,
  int const* __restrict__ pa,
  int* nm_ta,
  int* nm)
{
  for (auto tid = get_tid(); tid < assoc_size; tid += grid_stride()) { 
    // this thread was the first one to find this tetrahedron
    if (atomicCAS(nm_ta + ta[tid], -1, tid) == -1) {
      // we then want to nominate this point
      // but because we initialize pa to being all true, if any
      // entry was previously 0, we know it was marked false by
      // another thread so we set it back to being false
      if (atomicOr(nm + pa[tid], 1) == 0) {
        atomicAnd(nm + pa[tid], 0);
      }
    } else {
      atomicAnd(nm + pa[tid], 0);
    }
  }
}

__global__
void repair_nm_ta(
  int const assoc_size,
  int const* __restrict__ pa,
  int const* __restrict__ ta,
  int const* __restrict__ nm,
  int* __restrict__ nm_ta)
{
  for (auto tid = get_tid(); tid < assoc_size; tid += grid_stride()) {
    if (nm[pa[tid]] == 1) {
      nm_ta[ta[tid]] = tid;    
    }
  }
}