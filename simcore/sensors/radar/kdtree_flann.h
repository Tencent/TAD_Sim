/**
 * @file kdtree_flann.h
 * @author kekesong (kekesong@tencent.com)
 * @brief handle kdtree with flann
 * @version 0.1
 * @date 2024-03-19
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <flann/flann.hpp>
#include <memory>
#include <vector>

/**
 * @brief kdtree_flann
 *
 * @tparam Scalar type of data
 * @tparam idx_type type of indices
 * @tparam D dimension of data
 * @tparam Metric metric of flann
 */
template <typename Scalar, typename idx_type, int D = 2, typename Metric = flann::L2_Simple<Scalar>>
class kdtree_flann {
 public:
  using DistanceType = typename Metric::ResultType;
  template <typename T>
  struct Point {
    T p[D] = {0};

    explicit Point(const T *_d) { memcpy(p, _d, sizeof(T) * D); }
  };
  using Point_Scalar = Point<Scalar>;

  kdtree_flann() = default;
  virtual ~kdtree_flann() = default;

  /**
   * @brief Add point to kdtree
   *
   * @param p point
   * @param idx index of point
   */

  void Add(const Scalar p[], const idx_type &idx) {
    Point_Scalar qp(p);
    Add(qp, idx);
  }
  void Add(const Point_Scalar &p, const idx_type &idx) {
    for (int i = 0; i < D; i++) {
      xyzdata_.push_back(p.p[i]);
    }
    idxdata_.push_back(idx);
  }
  /**
   * @brief clear all points in the tree
   *
   */
  void Clear() {
    xyzdata_.clear();
    idxdata_.clear();
  }

  /**
   * @brief valid the tree
   *
   * @return true if valid
   * @return false if not valid
   */
  bool Valid() const {
    if (xyzdata_.empty()) return false;
    if (index_.get() == nullptr) return false;
    return true;
  }

  /**
   * @brief build the tree
   *
   * @return true on successful building
   * @return false on failure building
   */
  bool Build() {
    if (xyzdata_.empty()) return false;
    //-- Build Flann Matrix container (map to already allocated memory)
    datasetM_.reset(new flann::Matrix<Scalar>(const_cast<Scalar *>(xyzdata_.data()), xyzdata_.size() / D, D));

    //-- Build FLANN index
    index_.reset(new flann::Index<Metric>(*datasetM_, flann::KDTreeIndexParams(4)));
    index_->buildIndex();

    return true;
  }
  /**
   * @brief search neighbours of query point
   *
   * @param query point to search
   * @param pvec_indices get indexes of neighbours
   * @param pvec_distances get distances of neighbours
   * @param NN set maximum number of neighbours
   * @return true on successful search
   * @return false on failure searching
   */
  bool SearchNeighbours(const Scalar *query, std::vector<idx_type> &pvec_indices,
                        std::vector<DistanceType> &pvec_distances, size_t NN) const {
    Point_Scalar qp(query);
    return SearchNeighbours(qp, pvec_indices, pvec_distances, NN);
  }
  bool SearchNeighbours(const Point_Scalar &query, std::vector<idx_type> &pvec_indices,
                        std::vector<DistanceType> &pvec_distances, size_t NN) const {
    if (index_.get() == nullptr) return false;

    if (NN > datasetM_->rows) NN = datasetM_->rows;
    std::vector<DistanceType> vec_distances(NN);
    DistanceType *distancePTR = &(vec_distances[0]);
    flann::Matrix<DistanceType> dists(distancePTR, 1, NN);

    std::vector<size_t> vec_indices(NN, -1);
    flann::Matrix<size_t> indices(&(vec_indices[0]), 1, NN);

    flann::Matrix<Scalar> queries(const_cast<Scalar *>(&query.p[0]), 1, D);
    // do a knn search, using 128 checks
    flann::SearchParams params(std::max<int>(32, NN));

    if (index_->knnSearch(queries, indices, dists, NN, params) > 0) {
      // Save the resulting found indices
      pvec_indices.reserve(NN);
      pvec_distances.reserve(NN);

      for (size_t j = 0; j < NN; ++j) {
        pvec_indices.emplace_back(idxdata_.at(vec_indices[j]));
        pvec_distances.emplace_back(vec_distances[j]);
      }

      return true;
    } else {
      return false;
    }
    return false;
  }

  /**
   * @brief search nearest neighbour of query point
   *
   * @param query point to search
   * @param pvec_indice get index of nearest neighbour
   * @param pvec_distance get distance of nearest neighbour
   * @return true on successful search
   * @return false on failure searching
   */
  bool SearchNeighbour(const Point_Scalar &query, idx_type &pvec_indice, DistanceType &pvec_distance) const {
    std::vector<idx_type> pvec_indices;
    std::vector<DistanceType> pvec_distances;
    if (!SearchNeighbours(query, pvec_indices, pvec_distances, 1)) return false;
    pvec_indice = pvec_indices.front();
    pvec_distance = pvec_distances.front();
    return true;
  }

  /**
   * @brief search neighbors within given radius
   *
   * @param query point to search
   * @param pvec_indices get indexes of neighbors
   * @param pvec_distances get distances of neighbors
   * @param radius search radius
   * @return true on successful search
   * @return false on failure searching
   */

  bool SearchRadius(const Scalar query[], std::vector<idx_type> &pvec_indices,
                    std::vector<DistanceType> &pvec_distances, Scalar radius) const {
    Point_Scalar qp(query);
    return SearchRadius(qp, pvec_indices, pvec_distances, radius);
  }
  bool SearchRadius(const Point_Scalar &query, std::vector<idx_type> &pvec_indices,
                    std::vector<DistanceType> &pvec_distances, Scalar radius) const {
    if (index_.get() == nullptr) return false;

    std::vector<std::vector<size_t>> indices;
    std::vector<std::vector<DistanceType>> dists;
    flann::Matrix<Scalar> queries(const_cast<Scalar *>(&query.p[0]), 1, D);
    // do a knn search, using 128 checks
    static flann::SearchParams params(1024);
    int pn = index_->radiusSearch(queries, indices, dists, radius, params);
    if (params.checks < 102400) {
      for (int i = 0; i < 5; i++) {
        if (pn < params.checks * 2 / 3) break;
        params.checks *= 2;
        printf("kdtree_flann: raduis :checks increase to %d", params.checks);
        pn = index_->radiusSearch(queries, indices, dists, radius, params);
      }
    }
    if (pn > 0) {
      assert(indices.size() == 1);
      assert(dists.size() == 1);
      assert(dists[0].size() == indices[0].size());
      if (indices.size() != 1 || dists.size() != 1 || indices.front().size() != dists.front().size()) {
        return false;
      }
      // Save the resulting found indices
      auto nn = indices.front().size();
      pvec_indices.resize(nn);
      pvec_distances.resize(nn);

      for (size_t j = 0; j < nn; ++j) {
        pvec_indices.at(j) = idxdata_.at(indices.front().at(j));
        pvec_distances.at(j) = dists.front().at(j);
      }

      return true;
    }

    return false;
  }

 private:
  std::vector<Scalar> xyzdata_;
  std::vector<idx_type> idxdata_;

  std::unique_ptr<flann::Matrix<Scalar>> datasetM_;
  std::unique_ptr<flann::Index<Metric>> index_;
};
