#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>
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
  size_t num_points = 0;
  bool invert_colors = false;
};

struct Pixel {
  size_t row = 0;
  size_t col = 0;
  float color = 0.f;
};

PixelVect CreateTexture(const TextureConfig& conf);

void WriteToPng(const TextureConfig& conf, const std::span<const Pixel> pixels,
                const std::filesystem::path& outfile);

}  // namespace ctext
