#include "Texture.h"

#include <algorithm>
#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/typedefs.hpp>
#include <cmath>
#include <limits>
#include <random>
#include <span>
#include <unordered_map>
#include <utility>

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
  return std::sqrt(xterm + yterm);
}

static std::pair<float, float> DistToNearestTwoPoints(
    const Pixel& pixel, const std::span<const Point2D> points) {
  float mindist1 = std::numeric_limits<float>::max();
  float mindist2 = std::numeric_limits<float>::max();
  for (const auto& point : points) {
    float dist = Distance(pixel.row, pixel.col, point.x, point.y);
    if (dist < mindist2) {
      mindist2 = dist;
      if (mindist2 < mindist1) {
        std::swap(mindist2, mindist1);
      }
    }
  }
  return {mindist1, mindist2};
}

float DistToNearestPoint(const Pixel& pixel,
                         const std::span<const Point2D> points) {
  if (points.empty()) {
    return 1.f;
  }

  float mindist = std::numeric_limits<float>::max();
  for (const auto& point : points) {
    float dist = Distance(pixel.row, pixel.col, point.x, point.y);
    mindist = std::min(dist, mindist);
  }
  return mindist;
}

float DistToNearestTwoPointsDelta(const Pixel& pixel,
                                  const std::span<const Point2D> points) {
  if (points.size() <= 1) {
    return 1.f;
  }

  const auto mindists = DistToNearestTwoPoints(pixel, points);
  return (mindists.second - mindists.first);
}

float DistToNearestTwoPointsProduct(const Pixel& pixel,
                                    const std::span<const Point2D> points) {
  if (points.size() <= 1) {
    return 1.f;
  }

  const auto mindists = DistToNearestTwoPoints(pixel, points);
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
  float mindist = std::numeric_limits<float>::max();
  float maxdist = 0.f;
  for (size_t i = 0; i < conf.dim.height; ++i) {
    for (size_t j = 0; j < conf.dim.width; ++j) {
      pixels.push_back({.row = i, .col = j, .color = 0});

      float distance = conf.GetDist(pixels.back(), points);
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

}  // namespace ctext
