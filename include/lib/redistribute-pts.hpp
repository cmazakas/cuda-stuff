#ifndef REGULUS_LIB_REDISTRIBUTE_PTS_HPP_
#define REGULUS_LIB_REDISTRIBUTE_PTS_HPP_

#include <stdio.h>

#include <thrust/copy.h>

#include "../globals.hpp"
#include "../math/tetra.hpp"
#include "../math/point.hpp"
#include "../array.hpp"
#include "../stack-vector.hpp"

template <typename T>
__global__
void redistribute_pts(
  int const assoc_size,
  int const num_tetra,
  tetra const* __restrict__ mesh,
  point_t<T> const* __restrict__ pts,
  int const* __restrict__ nm,
  int const* __restrict__ nm_ta,
  int const* __restrict__ fl,
  int* pa,
  int* ta,
  int* la,
  int* num_redistributions)
{
  for (auto tid = get_tid(); tid < assoc_size; tid += grid_stride()) {
    // store a copy of the current ta value
    int const ta_id = ta[tid];
    int const pa_id = pa[tid];
    
    // we encode a tid such that nm[pa[tid]] == 1 in nm_ta at ta[tid]
    // i.e., if (nm[pa[tid]] == 1) then nm_ta[ta[tid]] == tid
    // and we want to test validity of nm_ta[ta[tid]] because we're going
    // in reverse
    int const pa_tid = nm_ta[ta_id];

    // this means the tetrahedron was not even written to
    if (pa_tid == -1) {
      return;
    }

    // this point was not ultimately nominated even though it wrote
    // this check my ultimately be unnecessary but for now I'm going
    // to be safe (I think it's necessary)
    if (nm[pa_tid] != 1) {
      return;
    }

    if (pa_tid == tid) {
      return;
    }

    // we now know that pa_tid is actually a valid tuple id!
    // invalidate this association
    ta[tid] = -1;
    pa[tid] = -1;
    la[tid] = -1;

    // we know that we wrote to mesh at ta_id and that we then wrote
    // past the end of the mesh at fl[tid] + { 0[, 1[, 2]] }
    stack_vector<int, 4> local_pa;
    stack_vector<int, 4> local_ta;
    stack_vector<int, 4> local_la;
    
    stack_vector<tetra, 4> tets;

    // load the tetrahedra onto the stack
    // I really wanna create a stack-based vector
    // to use for this though
    tets.push_back(mesh[ta_id]);
    local_ta.push_back(ta_id);
    local_pa.push_back(pa_id);
    
    int const fract_size = __popc(la[pa_tid]);
    int const mesh_offset = num_tetra + fl[pa_tid];
    
    for (int i = 0; i < (fract_size - 1); ++i) {
      tets.push_back(mesh[mesh_offset + i]);
      local_ta.push_back(mesh_offset + i);
      local_pa.push_back(pa_id);
    }

    // now we can begin testing each one
    point_t<T> const p = pts[pa_id];
    for (int i = 0; i < tets.size(); ++i) {
      tetra const t = tets[i];
      
      point_t<T> const a = pts[t.x];
      point_t<T> const b = pts[t.y];
      point_t<T> const c = pts[t.z];
      point_t<T> const d = pts[t.w];
      
      local_la.push_back(loc<T>(a, b, c, d, p));
    }

    // this is some manual clean-up
    // if the location code is -1, we should just
    // null this assocation out completely
    for (int i = 0; i < local_la.size(); ++i) {
      if (local_la[i] == -1) {
        local_pa[i] = -1;
        local_ta[i] = -1;
      }
    }

    // now we have to do a write-back to main memory
    int const assoc_offset = assoc_size + (4 * atomicAdd(num_redistributions, 1));
    
    thrust::copy(thrust::seq, local_pa.begin(), local_pa.end(), pa + assoc_offset);
    thrust::copy(thrust::seq, local_ta.begin(), local_ta.end(), ta + assoc_offset);
    thrust::copy(thrust::seq, local_la.begin(), local_la.end(), la + assoc_offset);
  }
}

#endif // REGULUS_LIB_REDISTRIBUTE_PTS_HPP_