#include <chrono>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <span>
#include <stdexcept>
#include <vector>

#include "Texture.h"
#include "Types.h"

struct DataPoint {
  size_t dim = 0;
  size_t num_points = 0;
  long elapsed_time_ms = 0;
};

static void WriteToCsv(const std::span<const DataPoint> data,
                       const std::filesystem::path& filepath) {
  std::ofstream fhandle(filepath);
  if (!fhandle) {
    throw std::runtime_error(
        std::format("unable to open file '{}' for writing", filepath.c_str()));
  }

  fhandle << "dim,npoints,elapsed_time_ms" << std::endl;
  for (const auto& point : data) {
    const auto fmt = std::format("{},{},{}", point.dim, point.num_points,
                                 point.elapsed_time_ms);
    fhandle << fmt << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "usage: benchmark OUTFILE" << std::endl;
    return 1;
  }

  // constexpr size_t kMaxDim = 2000;
  // constexpr size_t kDimStepSize = 100;
  // std::vector<ctext::Dimension2D> dimensions;
  // for (size_t i = kDimStepSize; i <= kMaxDim; i += kDimStepSize) {
  //   dimensions.push_back({.width = i, .height = i});
  // }

  std::vector<DataPoint> data;
  constexpr size_t kDim = 2000;
  constexpr size_t kPointStepSize = 10000;
  constexpr size_t kMaxPoints = 10000000;
  for (size_t i = kPointStepSize; i <= kMaxPoints; i += kPointStepSize) {
    ctext::TextureConfig conf = {
        .dim = {kDim},
        .num_points = i,
        .invert_colors = false,
        .is_tiled = false,
        .num_neighbors = 1,
        .op = ctext::DistOp::kAdd,
    };
    auto start = std::chrono::high_resolution_clock::now();
    auto pixels = ctext::CreateTexture(conf);
    auto stop = std::chrono::high_resolution_clock::now();

    auto delta_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(stop - start)
            .count();
    data.push_back({
        .dim = kDim,
        .num_points = i,
        .elapsed_time_ms = delta_ms,
    });
  }

  WriteToCsv(data, argv[1]);

  return 0;
}
