#pragma once

#include <filesystem>
#include <istream>
#include <span>

#include "Types.h"

namespace ctext {

namespace distfunc {

float DistToNearestPoint(const Pixel& pixel,
                         const std::span<const Point2D> points,
                         const TextureConfig& conf);
float DistToNearestTwoPointsDelta(const Pixel& pixel,
                                  const std::span<const Point2D> points,
                                  const TextureConfig& conf);
float DistToNearestTwoPointsProduct(const Pixel& pixel,
                                    const std::span<const Point2D> points,
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
