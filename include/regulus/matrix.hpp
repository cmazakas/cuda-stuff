#ifndef REGULUS_MATRIX_HPP_
#define REGULUS_MATRIX_HPP_

#include <type_traits>

#include <thrust/equal.h>
#include <thrust/execution_policy.h>

#include "regulus/array.hpp"
#include "regulus/vector.hpp"
#include "regulus/utils/equals.hpp"

namespace regulus
{
template <
  typename T,
  size_t R,
  size_t C,
  typename = typename std::enable_if<std::is_arithmetic<T>::value>::type
>
struct matrix
{
  using array_type     = array<T, R * C>;
  using size_type      = typename array_type::size_type;
  using value_type     = typename array_type::value_type;
  using iterator       = typename array_type::iterator;
  using const_iterator = typename array_type::const_iterator;

  array_type data_;

  __host__ __device__
  auto operator[](size_type const row_idx) -> iterator
  {
    return data_.begin() + (row_idx * C);
  }

  __host__ __device__
  auto operator[](size_type const row_idx) const -> const_iterator
  {
    return data_.begin() + (row_idx * C);
  }

  __host__ __device__
  auto operator==(matrix<T, R, C> const& other) const -> bool
  {
    return thrust::equal(
      thrust::seq,
      data_.begin(), data_.end(),
      other.data_.begin(),
      [](T const x, T const y) -> bool
      {
        return eq(x, y);
      });
  }

  __host__ __device__
  auto operator!=(matrix<T, R, C> const& other) const -> bool
  {
    return !(*this == other);
  }

  __host__ __device__
  auto size(void) const -> size_type
  {
    return data_.size();
  }

  __host__ __device__
  auto row(size_type const row_idx) const -> vector<T, C>
  {
    vector<T, C> row;

    auto const array_offset = row_idx * C;

    for (size_type i = 0; i < C; ++i) {
      row[i] = data_[array_offset + i];
    }

    return row;
  }

  __host__ __device__
  auto col(size_type const col_idx) const -> vector<T, R>
  {
    vector<T, R> col;

    for (size_type i = 0; i < R; ++i) {
      col[i] = data_[i * C + col_idx];
    }

    return col;
  }
};

}

#endif // REGULUS_MATRIX_HPP_