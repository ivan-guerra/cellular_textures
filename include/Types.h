#pragma once

#include <array>
#include <cstddef>
#include <functional>
#include <span>
#include <vector>

namespace ctext {

struct Pixel;
using PixelVect = std::vector<Pixel>;

struct Point2D;
using PointVect = std::vector<Point2D>;

struct TextureConfig;
using DistFunc = std::function<float(
    const Pixel&, const std::span<const Point2D>, const TextureConfig&)>;

constexpr size_t kTotalDistFuncs = 3;
using DistFuncTable = std::array<DistFunc, kTotalDistFuncs>;

enum DistMetric {
  DistToNearestPoint = 0,
  DistToNearestTwoPointsDelta,
  DistToNearestTwoPointsProduct
};

struct Dimension2D {
  size_t width = 0;
  size_t height = 0;
};

struct TextureConfig {
  Dimension2D dim;
  size_t num_points = 0;
  bool invert_colors = false;
  bool is_tiled = false;
  DistMetric metric = DistMetric::DistToNearestPoint;
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
