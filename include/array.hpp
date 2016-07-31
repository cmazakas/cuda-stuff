#ifndef REGULUS_ARRAY_HPP_
#define REGULUS_ARRAY_HPP_

namespace reg
{
  template <typename T, int N>
  struct array
  {
    using value_type = T;
    using size_type = int;
    using reference = value_type&;
    using iterator = value_type*;
    
    T data[N];
    
    __host__ __device__
    auto operator[](size_type const idx) -> reference
    {
      return data[idx];
    }
    
    __host__ __device__
    auto operator==(array<T, N> const& other) -> bool
    {
      bool not_equal = false;
      
      for (int i = 0; i < N; ++i) {
        not_equal = not_equal || (data[i] != other.data[i]);
      }
      
      return !not_equal;
    }
    
    __host__ __device__
    auto operator!=(array<T, N> const& other) -> bool
    {
      return !(*this == other);
    }
    
    __host__ __device__
    auto begin(void) -> iterator
    {
      return &(data[0]);
    }
    
    __host__ __device__
    auto end(void) -> iterator
    {
      return begin() + N;
    }
  };  
}


#endif // REGULUS_ARRAY_HPP_