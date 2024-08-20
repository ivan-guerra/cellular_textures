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

namespace ctext {

static double GetRandNum(int min, int max) {
  // https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distr(min, max);
  return distr(gen);
}

static double Distance(double x1, double y1, double x2, double y2) {
  double xterm = (x1 - x2) * (x1 - x2);
  double yterm = (y1 - y2) * (y1 - y2);
  return std::sqrtf(xterm + yterm);
}

static double DistanceWrapped(double x1, double y1, double x2, double y2,
                              const Dimension2D& dim) {
  double dx = std::abs(x1 - x2);
  double dy = std::abs(y1 - y2);
  if (dx > (dim.width / 2.0)) {
    dx = dim.width - dx;
  }
  if (dy > (dim.height / 2.0)) {
    dy = dim.height - dy;
  }
  return std::sqrt((dx * dx) + (dy * dy));
}

static std::pair<double, double> DistToNearestTwoPoints(
    const Pixel& pixel, const TwoDTree& points, const TextureConfig& conf) {
  const Point query = {static_cast<double>(pixel.row),
                       static_cast<double>(pixel.col)};
  std::vector<const Point*> result;
  points.knearest(query, 2, result);

  if (result.size() < 2) {
    throw std::runtime_error(
        std::format("could not find two points near target pixel ({}, {})",
                    pixel.row, pixel.col));
  }

  const Point* closest1 = result[0];
  const Point* closest2 = result[1];
  double mindist1 = 0.0;
  double mindist2 = 0.0;
  if (conf.is_tiled) {
    mindist1 = DistanceWrapped(pixel.row, pixel.col, closest1->x(),
                               closest1->y(), conf.dim);
    mindist2 = DistanceWrapped(pixel.row, pixel.col, closest2->x(),
                               closest2->y(), conf.dim);
  } else {
    mindist1 = Distance(pixel.row, pixel.col, closest1->x(), closest1->y());
    mindist2 = Distance(pixel.row, pixel.col, closest2->x(), closest2->y());
  }
  return {mindist1, mindist2};
}

double distfunc::DistToNearestPoint(const Pixel& pixel, const TwoDTree& points,
                                    const TextureConfig& conf) {
  const Point query = {static_cast<double>(pixel.row),
                       static_cast<double>(pixel.col)};
  std::vector<const Point*> result;
  points.knearest(query, 1, result);

  if (result.empty()) {
    throw std::runtime_error(
        std::format("could not find any points near target pixel ({}, {})",
                    pixel.row, pixel.col));
  }

  const Point* closest = result.front();
  if (conf.is_tiled) {
    return DistanceWrapped(pixel.row, pixel.col, closest->x(), closest->y(),
                           conf.dim);
  } else {
    return Distance(pixel.row, pixel.col, closest->x(), closest->y());
  }
}

double distfunc::DistToNearestTwoPointsDelta(const Pixel& pixel,
                                             const TwoDTree& points,
                                             const TextureConfig& conf) {
  const auto mindists = DistToNearestTwoPoints(pixel, points, conf);
  return std::abs(mindists.first - mindists.second);
}

double distfunc::DistToNearestTwoPointsProduct(const Pixel& pixel,
                                               const TwoDTree& points,
                                               const TextureConfig& conf) {
  const auto mindists = DistToNearestTwoPoints(pixel, points, conf);
  return (mindists.first * mindists.second);
}

PixelVect CreateTexture(const TextureConfig& conf) {
  TwoDTree tree;
  PointVect points(conf.num_points);
  for (size_t i = 0; i < conf.num_points; ++i) {
    points[i] = {GetRandNum(1, conf.dim.width - 1),
                 GetRandNum(1, conf.dim.height - 1)};
    tree.add(&points[i], &points[i]);
  }
  tree.build();

  using DistanceBuffer =
      std::unordered_map<size_t, std::unordered_map<size_t, double>>;
  DistanceBuffer distances;
  PixelVect pixels;
  double mindist = std::numeric_limits<double>::max();
  double maxdist = 0.0;
  for (size_t i = 0; i < conf.dim.height; ++i) {
    for (size_t j = 0; j < conf.dim.width; ++j) {
      pixels.push_back({.row = i, .col = j, .color = 0});

      double distance =
          distfunc::kFuncTable[conf.metric](pixels.back(), tree, conf);
      distances[i][j] = distance;
      mindist = std::min(distance, mindist);
      maxdist = std::max(distance, maxdist);
    }
  }

  for (Pixel& p : pixels) {
    const double color =
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
