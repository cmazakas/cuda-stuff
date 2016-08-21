#include "test-suite.hpp"
#include "../include/mesher.hpp"
#include "../include/domain.hpp"
#include "../include/math/tetra.hpp"
#include "../include/math/point.hpp"

auto mesher_tests(void) -> void
{
  std::cout << "Beginning mesher tests!" << std::endl;
  
  // We should be able to construct a mesh
  {
    using real = float;
    
    // grid of all points in x,y,z 0 through 9
    thrust::host_vector<point_t<real>> pts{ gen_cartesian_domain<real>(10) };
    pts.push_back(point_t<real>{0, 0, 0});
    pts.push_back(point_t<real>{100, 0, 0});
    pts.push_back(point_t<real>{0, 100, 0});
    pts.push_back(point_t<real>{0, 0, 100});
    
    int const a{(int ) pts.size() - 4};
    int const b{(int ) pts.size() - 3};
    int const c{(int ) pts.size() - 2};
    int const d{(int ) pts.size() - 1};
    
    tetra const root_tet{a, b, c, d};
    
    mesher<real> m{pts, root_tet};
    
    // do a quick check that all of our points are actually in the
    // root tetrahedron
    {
      auto const& pa = pts[root_tet.x];
      auto const& pb = pts[root_tet.y];
      auto const& pc = pts[root_tet.z];
      auto const& pd = pts[root_tet.w];
      
      assert(orient<real>(pa, pb, pc, pd) == orientation::positive);
      
      for (int i = 0; i < 10 * 10 * 10; ++i) {
        assert(loc<real>(pa, pb, pc, pd, pts[i]) != 255);      
      }
    }
    
    // pray for the best!
    m.triangulate();
  }
  
  std::cout << "Tests Passed!\n" << std::endl;
}