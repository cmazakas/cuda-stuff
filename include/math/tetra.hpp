#ifndef REGULUS_TETRA_HPP_
#define REGULUS_TETRA_HPP_

#include "../common.hpp"
#include "../array.hpp"
#include "matrix.hpp"
#include "equals.hpp"

enum class orientation { positive = 1, zero = 0, negative = 2 };

// Routine that calculates whether the point d
// is above the triangle spanned by abc
template <typename T>
__host__ __device__
auto orient(
  point_t<T> const& a,
  point_t<T> const& b,
  point_t<T> const& c,
  point_t<T> const& d)
-> orientation
{ 
  matrix<T, 4, 4> const m{ 1, a.x, a.y, a.z,
                           1, b.x, b.y, b.z,
                           1, c.x, c.y, c.z,
                           1, d.x, d.y, d.z };

  auto const det = m.det();
  auto const not_equal_to_zero = !eq<T>(det, 0.0);
  
  if (det > 0.0 && not_equal_to_zero) {
    return orientation::positive;
    
  } else if (!not_equal_to_zero) {
    return orientation::zero;
    
  } else {
    return orientation::negative;
  }
}

// Calculate the magnitude of a point (i.e. vector)
template <typename T>
__host__ __device__
auto mag(point_t<T> const& p) -> T
{
  return (
    p.x * p.x +
    p.y * p.y +
    p.z * p.z);
}

// Function that calculates whether or not p is contained
// in the sphere circumscribed by the tetrahedron abcd
template <typename T>
__host__ __device__
auto insphere(
  point_t<T> const& a,
  point_t<T> const& b,
  point_t<T> const& c,
  point_t<T> const& d,
  point_t<T> const& p)
-> orientation
{
  matrix<T, 5, 5> const m{
    1.0, a.x, a.y, a.z, mag<T>(a),
    1.0, b.x, b.y, b.z, mag<T>(b),
    1.0, c.x, c.y, c.z, mag<T>(c),
    1.0, d.x, d.y, d.z, mag<T>(d),
    1.0, p.x, p.y, p.z, mag<T>(p) };
    
  auto const det = m.det();
  auto const not_equal_to_zero = !eq<T>(det, 0.0);
  
  if (det > 0.0 && not_equal_to_zero) {
    return orientation::positive;
    
  } else if (!not_equal_to_zero) {
    return orientation::zero;
    
  } else {
    return orientation::negative;
  } 
}

// Function that builds a location code which is a bitwise
// encoding of a point's location relative to the tetrahedron
// spanned by abcd
//
// bit value of 0 = orientation::zero
// bit value of 1 = orientation::positive
// loc() == 255? then point is outside abcd
//
// All faces are positively oriented
// bit loc 0 = face 0 => 321 
// bit loc 1 = face 1 => 023
// bit loc 2 = face 2 => 031
// bit loc 3 = face 3 => 012
template <typename T>
__host__ __device__
auto loc(
  point_t<T> const& a,
  point_t<T> const& b,
  point_t<T> const& c,
  point_t<T> const& d,
  point_t<T> const& p)
-> unsigned char
{
  int const face_ids[12] = { 3, 2, 1,
                             0, 2, 3,
                             0, 3, 1,
                             0, 1, 2 };
  
  int const num_pts{4};
  array<point_t<T>, num_pts> const pts{a, b, c, d};
  
  unsigned char loc{0};
  
  for (int i = 0; i < num_pts; ++i) {
    orientation const ort{orient<T>(
      pts[face_ids[i * 3 + 0]],
      pts[face_ids[i * 3 + 1]],
      pts[face_ids[i * 3 + 2]],
      p)};
    
    if (ort == orientation::negative) {
      return 255;
    }
    
    loc |= (static_cast<unsigned int>(ort) << i);
  }
  
  return loc;
}

#endif // REGULUS_TETRA_HPP_