#pragma once

#include <filesystem>
#include <istream>
#include <span>

#include "Types.h"

namespace ctext {

enum DistOp : char {
  kAdd = '+',
  kSubtract = '-',
  kMultiply = '*',
};

struct TextureConfig {
  Dimension2D dim;
  size_t num_points = 0;
  bool invert_colors = false;
  bool is_tiled = false;
  size_t num_neighbors = 0;
  DistOp op;
};

PixelVect CreateTexture(const TextureConfig& conf);

void WriteToPng(const TextureConfig& conf, const std::span<const Pixel> pixels,
                const std::filesystem::path& outfile);

std::istream& operator>>(std::istream& in, DistOp& metric);

}  // namespace ctext
