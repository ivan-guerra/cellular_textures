#pragma once

#include <array>
#include <filesystem>
#include <functional>
#include <istream>
#include <span>

#include "TwoDTree.h"
#include "Types.h"

namespace ctext {

struct TextureConfig;
using DistFunc =
    std::function<float(const Pixel&, const TwoDTree&, const TextureConfig&)>;

constexpr size_t kTotalDistFuncs = 3;
using DistFuncTable = std::array<DistFunc, kTotalDistFuncs>;

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

namespace distfunc {

float DistToNearestPoint(const Pixel& pixel, const TwoDTree& points,
                         const TextureConfig& conf);
float DistToNearestTwoPointsDelta(const Pixel& pixel, const TwoDTree& points,
                                  const TextureConfig& conf);
float DistToNearestTwoPointsProduct(const Pixel& pixel, const TwoDTree& points,
                                    const TextureConfig& conf);

const DistFuncTable kFuncTable = {
    DistToNearestPoint,
    DistToNearestTwoPointsDelta,
    DistToNearestTwoPointsProduct,
};

}  // namespace distfunc

PixelVect CreateTexture(const TextureConfig& conf);

void WriteToPng(const TextureConfig& conf, const std::span<const Pixel> pixels,
                const std::filesystem::path& outfile);

std::istream& operator>>(std::istream& in, DistMetric& metric);

}  // namespace ctext
