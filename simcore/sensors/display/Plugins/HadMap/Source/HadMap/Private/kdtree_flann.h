#pragma once
#include <vector>
#include <flann/flann.hpp>

template <typename Scalar, typename idx_type, typename Metric = flann::L2_Simple<Scalar>>
class kdtree_flann
{
public:
    using DistanceType = typename Metric::ResultType;
    template <typename T>
    struct Point3
    {
        T x, y, z;

        Point3() : x(0.0), y(0.0), z(0.0)
        {
        }

        Point3(T _x, T _y, T _z) : x(_x), y(_y), z(_z)
        {
        }
        Point3(const T* _d) : x(_d[0]), y(_d[1]), z(_d[2])
        {
        }
    };
    using Point3_Scalar = Point3<Scalar>;

    kdtree_flann() = default;
    virtual ~kdtree_flann() = default;

    void Add(const Point3_Scalar& p, const idx_type& idx)
    {
        xyzdata_.push_back(p.x);
        xyzdata_.push_back(p.y);
        xyzdata_.push_back(p.z);
        idxdata_.push_back(idx);
    }
    void Clear()
    {
        xyzdata_.clear();
        idxdata_.clear();
    }

    bool Valid() const
    {
        if (xyzdata_.empty())
            return false;
        if (index_.get() == nullptr)
            return false;
        return true;
    }

    bool Build()
    {
        if (xyzdata_.empty())
            return false;
        //-- Build Flann Matrix container (map to already allocated memory)
        datasetM_.reset(new flann::Matrix<Scalar>((Scalar*) xyzdata_.data(), xyzdata_.size() / 3, 3));

        //-- Build FLANN index
        index_.reset(new flann::Index<Metric>(*datasetM_, flann::KDTreeIndexParams(4)));
        index_->buildIndex();

        return true;
    }
    bool SearchNeighbours(const Point3_Scalar& query, std::vector<idx_type>& pvec_indices,
        std::vector<DistanceType>& pvec_distances, size_t NN) const
    {
        if (index_.get() == nullptr)
            return false;

        if (NN > datasetM_->rows)
            NN = datasetM_->rows;
        std::vector<DistanceType> vec_distances(NN);
        DistanceType* distancePTR = &(vec_distances[0]);
        flann::Matrix<DistanceType> dists(distancePTR, 1, NN);

        std::vector<size_t> vec_indices(NN, -1);
        flann::Matrix<size_t> indices(&(vec_indices[0]), 1, NN);

        flann::Matrix<Scalar> queries((Scalar*) &query, 1, 3);
        // do a knn search, using 128 checks
        flann::SearchParams params(128);

        if (index_->knnSearch(queries, indices, dists, NN, params) > 0)
        {
            // Save the resulting found indices
            pvec_indices.reserve(NN);
            pvec_distances.reserve(NN);

            for (size_t j = 0; j < NN; ++j)
            {
                pvec_indices.emplace_back(idxdata_.at(vec_indices[j]));
                pvec_distances.emplace_back(vec_distances[j]);
            }

            return true;
        }
        else
        {
            return false;
        }
        return false;
    }

    bool SearchNeighbour(const Point3_Scalar& query, idx_type& pvec_indice, DistanceType& pvec_distance) const
    {
        std::vector<idx_type> pvec_indices;
        std::vector<DistanceType> pvec_distances;
        if (!SearchNeighbours(query, pvec_indices, pvec_distances, 1))
            return false;
        pvec_indice = pvec_indices.front();
        pvec_distance = pvec_distances.front();
        return true;
    }

private:
    std::vector<Scalar> xyzdata_;
    std::vector<idx_type> idxdata_;

    std::unique_ptr<flann::Matrix<Scalar>> datasetM_;
    std::unique_ptr<flann::Index<Metric>> index_;
};
