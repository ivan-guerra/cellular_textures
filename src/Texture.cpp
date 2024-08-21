#include "Texture.h"

#include <algorithm>
#include <boost/gil.hpp>
#include <boost/gil/extension/io/png.hpp>
#include <boost/gil/typedefs.hpp>
#include <cmath>
#include <cstdlib>
#include <format>
#include <istream>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>

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

static double ApplyOp(double a, double b, DistOp op) {
  switch (op) {
    case DistOp::kAdd:
      return (a + b);
    case DistOp::kSubtract:
      return std::abs(a - b);
    case DistOp::kMultiply:
      return (a * b);
    default:
      throw std::runtime_error("unknown dist op");
  }
}

static double DistToNearestKPoints(const Pixel& pixel, const TwoDTree& points,
                                   const TextureConfig& conf) {
  const Point query = {static_cast<double>(pixel.row),
                       static_cast<double>(pixel.col)};
  std::vector<const Point*> search_result;
  points.knearest(query, conf.num_neighbors, search_result);

  if (search_result.size() < conf.num_neighbors) {
    throw std::runtime_error(
        std::format("could not find {} points near target pixel ({}, {})",
                    conf.num_neighbors, pixel.row, pixel.col));
  }

  double result = 1.0;
  double curr_dist = 0.0;
  for (const Point* p : search_result) {
    if (conf.is_tiled) {
      curr_dist =
          DistanceWrapped(pixel.row, pixel.col, p->x(), p->y(), conf.dim);
    } else {
      curr_dist = Distance(pixel.row, pixel.col, p->x(), p->y());
    }
    result = ApplyOp(result, curr_dist, conf.op);
  }
  return result;
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

      double distance = DistToNearestKPoints(pixels.back(), tree, conf);
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

std::istream& operator>>(std::istream& in, DistOp& metric) {
  std::string token;
  in >> token;
  if (token == "+") {
    metric = DistOp::kAdd;
  } else if (token == "-") {
    metric = DistOp::kSubtract;
  } else if (token == "*") {
    metric = DistOp::kMultiply;
  } else {
    in.setstate(std::ios_base::failbit);
  }
  return in;
}

}  // namespace ctext
