#pragma once

#include <cstddef>
#include <vector>

namespace ctext {

struct Pixel;
using PixelVect = std::vector<Pixel>;

struct Point2D;
using PointVect = std::vector<Point2D>;

struct Dimension2D {
  size_t width = 0;
  size_t height = 0;
};

struct Pixel {
  size_t row = 0;
  size_t col = 0;
  float color = 0.f;
};

struct Point2D {
  float x = 0.f;
  float y = 0.f;
};

}  // namespace ctext
