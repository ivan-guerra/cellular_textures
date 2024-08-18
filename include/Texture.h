#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <functional>
#include <span>
#include <vector>

namespace ctext {

struct Pixel;
using PixelVect = std::vector<Pixel>;

struct Point2D;
using PointVect = std::vector<Point2D>;

using DistFunc =
    std::function<float(const Pixel&, const std::span<const Point2D>)>;

struct Dimension2D {
  size_t width = 0;
  size_t height = 0;
};

struct TextureConfig {
  Dimension2D dim;
  size_t num_points = 0;
  bool invert_colors = false;
  DistFunc GetDist;
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

float DistToNearestPoint(const Pixel& pixel,
                         const std::span<const Point2D> points);
float DistToNearestTwoPointsDelta(const Pixel& pixel,
                                  const std::span<const Point2D> points);
float DistToNearestTwoPointsProduct(const Pixel& pixel,
                                    const std::span<const Point2D> points);

PixelVect CreateTexture(const TextureConfig& conf);

void WriteToPng(const TextureConfig& conf, const std::span<const Pixel> pixels,
                const std::filesystem::path& outfile);

}  // namespace ctext
