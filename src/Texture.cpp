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

namespace ctext {

static int GetRandNum(int min, int max) {
  // https://stackoverflow.com/questions/7560114/random-number-c-in-some-range
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distr(min, max);
  return distr(gen);
}

static float DistToNearestPoint(const Pixel& pixel,
                                const std::span<const float> xcoords,
                                const std::span<const float> ycoords,
                                size_t num_points) {
  float mindist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < num_points; ++i) {
    float x2 = (xcoords[i] - pixel.row) * (xcoords[i] - pixel.row);
    float y2 = (ycoords[i] - pixel.col) * (ycoords[i] - pixel.col);
    float dist = std::sqrt(x2 + y2);
    mindist = std::min(dist, mindist);
  }
  return mindist;
}

PixelVect CreateTexture(const TextureConfig& conf) {
  std::vector<float> xcoords(conf.num_points, 0.f);
  std::vector<float> ycoords(conf.num_points, 0.f);
  for (size_t i = 0; i < conf.num_points; ++i) {
    xcoords[i] = GetRandNum(1, conf.dim.width - 1);
    ycoords[i] = GetRandNum(1, conf.dim.height - 1);
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

      float distance =
          DistToNearestPoint(pixels.back(), xcoords, ycoords, conf.num_points);
      // TODO(ieg): The formula used to populate the distance buffer controls
      // much of how the final texture looks. A future revision should make this
      // formula configurable.
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
