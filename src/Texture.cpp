#include "Texture.h"

#include <algorithm>
#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/typedefs.hpp>
#include <cmath>
#include <cstdlib>
#include <format>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "TwoDTree.h"

namespace ctext {

static int GetRandNum(int min, int max) {
  // https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(min, max);
  return distr(gen);
}

static float Distance(float x1, float y1, float x2, float y2) {
  float xterm = (x1 - x2) * (x1 - x2);
  float yterm = (y1 - y2) * (y1 - y2);
  return std::sqrtf(xterm + yterm);
}

static float DistanceWrapped(float x1, float y1, float x2, float y2,
                             const Dimension2D& dim) {
  float dx = std::abs(x1 - x2);
  float dy = std::abs(y1 - y2);
  if (dx > (dim.width / 2.f)) {
    dx = dim.width - dx;
  }
  if (dy > (dim.height / 2.f)) {
    dy = dim.height - dy;
  }
  return std::sqrt((dx * dx) + (dy * dy));
}

static std::pair<float, float> DistToNearestTwoPoints(
    const Pixel& pixel, const TwoDTree& points, const TextureConfig& conf) {
  const auto nearest_neighbors = points.FindNNearestNeighbors(
      {
          .x = static_cast<float>(pixel.row),
          .y = static_cast<float>(pixel.col),
      },
      2);

  if (nearest_neighbors.size() < 2) {
    throw std::runtime_error(std::format(
        "could not find two points near target pixel, row={} col={}", pixel.row,
        pixel.col));
  }

  const Point2D& closest1 = nearest_neighbors[0];
  const Point2D& closest2 = nearest_neighbors[1];
  float mindist1 = 0.f;
  float mindist2 = 0.f;
  if (conf.is_tiled) {
    mindist1 =
        DistanceWrapped(pixel.row, pixel.col, closest1.x, closest1.y, conf.dim);
    mindist2 =
        DistanceWrapped(pixel.row, pixel.col, closest2.x, closest2.y, conf.dim);
  } else {
    mindist1 = Distance(pixel.row, pixel.col, closest1.x, closest1.y);
    mindist2 = Distance(pixel.row, pixel.col, closest2.x, closest2.y);
  }
  return {mindist1, mindist2};
}

float distfunc::DistToNearestPoint(const Pixel& pixel, const TwoDTree& points,
                                   const TextureConfig& conf) {
  const auto nearest_neighbors = points.FindNNearestNeighbors(
      {
          .x = static_cast<float>(pixel.row),
          .y = static_cast<float>(pixel.col),
      },
      1);

  if (nearest_neighbors.empty()) {
    throw std::runtime_error(std::format(
        "could not find any points near target pixel, row={} col={}", pixel.row,
        pixel.col));
  }

  const Point2D& closest = nearest_neighbors.front();
  if (conf.is_tiled) {
    return DistanceWrapped(pixel.row, pixel.col, closest.x, closest.y,
                           conf.dim);
  } else {
    return Distance(pixel.row, pixel.col, closest.x, closest.y);
  }
}

float distfunc::DistToNearestTwoPointsDelta(const Pixel& pixel,
                                            const TwoDTree& points,
                                            const TextureConfig& conf) {
  const auto mindists = DistToNearestTwoPoints(pixel, points, conf);
  return std::fabs(mindists.first - mindists.second);
}

float distfunc::DistToNearestTwoPointsProduct(const Pixel& pixel,
                                              const TwoDTree& points,
                                              const TextureConfig& conf) {
  const auto mindists = DistToNearestTwoPoints(pixel, points, conf);
  return (mindists.first * mindists.second);
}

PixelVect CreateTexture(const TextureConfig& conf) {
  PointVect points(conf.num_points);
  for (size_t i = 0; i < conf.num_points; ++i) {
    points[i].x = GetRandNum(1, conf.dim.width - 1);
    points[i].y = GetRandNum(1, conf.dim.height - 1);
  }

  using DistanceBuffer =
      std::unordered_map<size_t, std::unordered_map<size_t, float>>;
  DistanceBuffer distances;
  PixelVect pixels;
  TwoDTree tree(points);
  float mindist = std::numeric_limits<float>::max();
  float maxdist = 0.f;
  for (size_t i = 0; i < conf.dim.height; ++i) {
    for (size_t j = 0; j < conf.dim.width; ++j) {
      pixels.push_back({.row = i, .col = j, .color = 0});

      float distance =
          distfunc::kFuncTable[conf.metric](pixels.back(), tree, conf);
      distances[i][j] = distance;
      mindist = std::min(distance, mindist);
      maxdist = std::max(distance, maxdist);
    }
  }

  for (Pixel& p : pixels) {
    const float color =
        (distances[p.row][p.col] - mindist) / (maxdist - mindist);
    p.color = (conf.invert_colors) ? (1 - color) : color;
  }

  return pixels;
}

void WriteToPng(const TextureConfig& conf, const std::span<const Pixel> pixels,
                const std::filesystem::path& outfile) {
  boost::gil::gray8_image_t img(conf.dim.width, conf.dim.height);

  constexpr auto kGrayscaleMax = 255u;
  auto output_view = boost::gil::view(img);
  for (const Pixel& p : pixels) {
    output_view(p.col, p.row) =
        boost::gil::gray8_pixel_t(p.color * kGrayscaleMax);
  }

  boost::gil::write_view(outfile, boost::gil::const_view(img),
                         boost::gil::png_tag{});
}

std::istream& operator>>(std::istream& in, DistMetric& metric) {
  std::string token;
  in >> token;
  if (token == "0") {
    metric = DistMetric::DistToNearestPoint;
  } else if (token == "1") {
    metric = DistMetric::DistToNearestTwoPointsDelta;
  } else if (token == "2") {
    metric = DistMetric::DistToNearestTwoPointsProduct;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

}  // namespace ctext
