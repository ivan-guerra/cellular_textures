#pragma once

#include <cstddef>
#include <expected>
#include <vector>

namespace ctext {

struct Pixel;
using PixelVect = std::vector<Pixel>;

struct Dimension2D {
  size_t width = 0;
  size_t height = 0;
};

struct TextureConfig {
  Dimension2D dim;
  size_t num_points;
};

struct Pixel {
  size_t row = 0;
  size_t col = 0;
  float color = 0.f;
};

PixelVect CreateTexture(const TextureConfig& conf);

}  // namespace ctext
