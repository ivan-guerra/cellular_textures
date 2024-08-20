#include "TwoDTree.h"

#include <algorithm>
#include <format>
#include <limits>
#include <memory>
#include <stdexcept>

namespace ctext {

float TwoDTree::GetDimVal(const Point2D& p, Axis dim) const {
  if (Axis::X == dim) {
    return p.x;
  } else if (Axis::Y == dim) {
    return p.y;
  }
  throw std::runtime_error("unknown dimension value");
}

float TwoDTree::GetDistanceSquared(const Point2D& a, const Point2D& b) const {
  float xterm = a.x - b.x;
  float yterm = a.y - b.y;
  return (xterm * xterm) + (yterm * yterm);
}

int TwoDTree::FindAxisMedian(int l, int r, int depth, PointVect& points) {
  auto CompareXAxis = [](const Point2D& a, const Point2D& b) {
    return (a.x < b.x);
  };
  auto CompareYAxis = [](const Point2D& a, const Point2D& b) {
    return (a.y < b.y);
  };

  const int axis = depth % kDimensions;
  if (Axis::X == axis) {
    std::sort(points.begin() + l, points.begin() + r, CompareXAxis);
  } else if (Axis::Y == axis) {
    std::sort(points.begin() + l, points.begin() + r, CompareYAxis);
  } else {
    throw std::runtime_error(std::format("invalid axis value {}", axis));
  }
  return ((l + r) / 2);
}

TwoDTree::NodePtr TwoDTree::ConstructTree(int l, int r, int depth,
                                          PointVect& points) {
  if (l > r) {
    return nullptr;
  }

  int median = FindAxisMedian(l, r, depth, points);
  NodePtr node = std::make_unique<Node>(
      points[median], ConstructTree(l, median - 1, depth + 1, points),
      ConstructTree(median + 1, r, depth + 1, points));

  return node;
}

void TwoDTree::FindNNearestNeighbors(const NodePtr& node, const Point2D& query,
                                     size_t n, PointHeap& closest,
                                     float& min_dist, int depth) const {
  if (!node) {
    return;
  }

  const float dist = GetDistanceSquared(query, node->pos);
  if (closest.size() < n) {
    closest.push({dist, node->pos});
  } else {
    if (dist < closest.top().first) {
      closest.pop();
      closest.push({dist, node->pos});
    }
  }

  min_dist = closest.top().first;

  const Axis dim = static_cast<Axis>(depth % kDimensions);
  if (GetDimVal(query, dim) < GetDimVal(node->pos, dim)) {
    FindNNearestNeighbors(node->left, query, n, closest, min_dist, depth + 1);
    if ((GetDimVal(query, dim) + min_dist) >= GetDimVal(node->pos, dim)) {
      FindNNearestNeighbors(node->right, query, n, closest, min_dist,
                            depth + 1);
    }
  } else {
    FindNNearestNeighbors(node->right, query, n, closest, min_dist, depth + 1);
    if ((GetDimVal(query, dim) - min_dist) <= GetDimVal(node->pos, dim)) {
      FindNNearestNeighbors(node->left, query, n, closest, min_dist, depth + 1);
    }
  }
}

TwoDTree::TwoDTree(PointVect points) : root_(nullptr) {
  const int depth = 0;
  const int median = FindAxisMedian(0, points.size(), depth, points);
  root_ = std::make_unique<Node>(
      points[median], ConstructTree(0, median - 1, depth + 1, points),
      ConstructTree(median + 1, points.size() - 1, depth + 1, points));
}

PointVect TwoDTree::FindNNearestNeighbors(const Point2D& query,
                                          size_t n) const {
  if (!n) {
    throw std::runtime_error("invalid query for 0 nearest neighbors");
  }

  PointHeap closest;
  float min_dist = std::numeric_limits<float>::max();
  FindNNearestNeighbors(root_, query, n, closest, min_dist, 0);

  PointVect closest_neighbors;
  while (!closest.empty()) {
    closest_neighbors.push_back(closest.top().second);
    closest.pop();
  }
  return closest_neighbors;
}

}  // namespace ctext
