#pragma once

#include <memory>
#include <queue>
#include <utility>

#include "Types.h"

namespace ctext {

using PointHeapItem = std::pair<float, Point2D>;

class PointHeapItemCmp {
 public:
  bool operator()(const PointHeapItem& a, const PointHeapItem& b) {
    return (a.first < b.first);
  }
};

class TwoDTree {
 public:
  TwoDTree() = delete;
  TwoDTree(PointVect points);

  PointVect FindNNearestNeighbors(const Point2D& query, size_t n) const;

 private:
  struct Node;
  using NodePtr = std::unique_ptr<Node>;
  using PointHeap =
      std::priority_queue<PointHeapItem, std::vector<PointHeapItem>,
                          PointHeapItemCmp>;

  static constexpr int kDimensions = 2;

  enum Axis {
    X = 0,
    Y,
  };

  struct Node {
    Point2D pos;
    NodePtr left = nullptr;
    NodePtr right = nullptr;

    Node(const Point2D& pos_, NodePtr left_, NodePtr right_)
        : pos(pos_), left(std::move(left_)), right(std::move(right_)) {}
  };

  float GetDimVal(const Point2D& p, Axis dim) const;

  float GetDistanceSquared(const Point2D& a, const Point2D& b) const;

  int FindAxisMedian(int l, int r, int depth, PointVect& points);

  NodePtr ConstructTree(int l, int r, int depth, PointVect& points);

  void FindNNearestNeighbors(const NodePtr& root, const Point2D& query,
                             size_t n, PointHeap& closest, float& min_dist,
                             int depth) const;

  NodePtr root_;
};

}  // namespace ctext
