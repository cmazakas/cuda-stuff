#include <thrust/extrema.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/execution_policy.h>

#include "regulus/array.hpp"
#include "regulus/algorithm/location.hpp"
#include "regulus/algorithm/nominate.hpp"

#include <catch.hpp>

TEST_CASE("Nominating points...")
{
  SECTION("should work as expected")
  {
    auto const assoc_size = size_t{11};

    using array_t = regulus::array<ptrdiff_t, assoc_size>;

    auto const ta_data = array_t{0, 1, 2, 3, 2, 5, 6, 7, 8, 1, 8};
    auto const pa_data = array_t{0, 0, 0, 0, 2, 2, 3, 3, 3, 4, 4};

    auto ta = thrust::device_vector<ptrdiff_t>{ta_data.begin(), ta_data.end()};
    auto pa = thrust::device_vector<ptrdiff_t>{pa_data.begin(), pa_data.end()};
    auto la = thrust::device_vector<regulus::loc_t>{assoc_size, regulus::outside_v};
    auto nm = thrust::device_vector<bool>{
      static_cast<size_t>(1 + *thrust::max_element(pa_data.begin(), pa_data.end())),
      false};

    regulus::nominate(assoc_size, pa, ta, la, nm);
    cudaDeviceSynchronize();

    auto h_ta = thrust::host_vector<ptrdiff_t>{ta};
    auto h_pa = thrust::host_vector<ptrdiff_t>{pa};
    auto h_nm = thrust::host_vector<bool>{nm};

    auto nominated_cnt = thrust::host_vector<unsigned>{
      static_cast<size_t>(1 + *thrust::max_element(h_ta.begin(), h_ta.end())), 0};

    auto found_duplicate = false;
    auto num_nominated   = int{0};

    for (size_t i = 0; i < assoc_size; ++i) {
      if (h_nm[h_pa[i]]) {
        if (nominated_cnt[h_ta[i]] > 1) {
          found_duplicate = true;
        }
        ++num_nominated;
      }
    }

    REQUIRE(found_duplicate == false);
    REQUIRE(num_nominated > 0);
  }
}