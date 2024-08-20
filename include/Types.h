#pragma once

#include <boost/geometry/geometries/point_xy.hpp>
#include <cstddef>
#include <vector>

#include "kdtree.h"

namespace ctext {

using Point = boost::geometry::model::d2::point_xy<double>;
using TwoDTree = spatial_index::kdtree<Point>;
struct Pixel;
using PixelVect = std::vector<Pixel>;
using PointVect = std::vector<Point>;

struct Dimension2D {
  size_t width = 0;
  size_t height = 0;
};

struct Pixel {
  size_t row = 0;
  size_t col = 0;
  double color = 0.0;
};

}  // namespace ctext
