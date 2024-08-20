#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <istream>
#include <span>

#include "Types.h"

namespace ctext {

enum DistMetric {
  DistToNearestPoint = 0,
  DistToNearestTwoPointsDelta,
  DistToNearestTwoPointsProduct
};

struct TextureConfig {
  Dimension2D dim;
  size_t num_points = 0;
  bool invert_colors = false;
  bool is_tiled = false;
  DistMetric metric = DistMetric::DistToNearestPoint;
};

PixelVect CreateTexture(const TextureConfig& conf);

void WriteToPng(const TextureConfig& conf, const std::span<const Pixel> pixels,
                const std::filesystem::path& outfile);

std::istream& operator>>(std::istream& in, DistMetric& metric);

namespace distfunc {

constexpr size_t kTotalDistFuncs = 3;

using DistFunc =
    std::function<double(const Pixel&, const TwoDTree&, const TextureConfig&)>;
using DistFuncTable = std::array<DistFunc, kTotalDistFuncs>;

double DistToNearestPoint(const Pixel& pixel, const TwoDTree& points,
                          const TextureConfig& conf);
double DistToNearestTwoPointsDelta(const Pixel& pixel, const TwoDTree& points,
                                   const TextureConfig& conf);
double DistToNearestTwoPointsProduct(const Pixel& pixel, const TwoDTree& points,
                                     const TextureConfig& conf);

const DistFuncTable kFuncTable = {
    DistToNearestPoint,
    DistToNearestTwoPointsDelta,
    DistToNearestTwoPointsProduct,
};

}  // namespace distfunc

}  // namespace ctext
