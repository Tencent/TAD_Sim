// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCore>
#include "tx_od_marco.h"
#include "tx_type_defs.h"

TX_NAMESPACE_OPEN(Base)

#if UseDouble
using txVec2I = Eigen::Vector2i;
using txVec3I = Eigen::Vector3i;
using txVec4I = Eigen::Vector4i;
using txVecI = Eigen::VectorXi;
using txVec2 = Eigen::Vector2d;
using txVec3 = Eigen::Vector3d;
using txVec4 = Eigen::Vector4d;
using txVec = Eigen::VectorXd;
using txMat2 = Eigen::Matrix2d;
using txMat3 = Eigen::Matrix3d;
using txMat4 = Eigen::Matrix4d;
using txMat = Eigen::MatrixXd;
using txSpMat = Eigen::SparseMatrix<txFloat, Eigen::RowMajor>;
using Vertex3 = txVec3;
#else
using txVec2I = Eigen::Vector2i;
using txVec3I = Eigen::Vector3i;
using txVec4I = Eigen::Vector4i;
using txVecI = Eigen::VectorXi;
using txVec2 = Eigen::Vector2f;
using txVec3 = Eigen::Vector3f;
using txVec4 = Eigen::Vector4f;
using txVec = Eigen::VectorXf;
using txMat2 = Eigen::Matrix2f;
using txMat3 = Eigen::Matrix3f;
using txMat4 = Eigen::Matrix4f;
using txMat = Eigen::MatrixXf;
using txSpMat = Eigen::SparseMatrix<txFloat, Eigen::RowMajor>;
using Vertex3 = txVec3;
#endif

TX_NAMESPACE_CLOSE(Base)

TX_NAMESPACE_OPEN(hadmap)

// point
// class txPoint
//{
//    using txFloat = Base::txFloat;
// public:
//    // x - global | lon - wgs84
//    txFloat          x = 0.0;
//    // y - global | lat - wgs84
//    txFloat          y = 0.0;
//    // z - global | ele - wgs84
//    txFloat          z = 0.0;
// public:
//    // construct operation
//    txPoint() : x(0.0), y(0.0), z(0.0) {}
//
//    txPoint(txFloat px, txFloat py, txFloat pz)
//        : x(px), y(py), z(pz) {}
//
//    txPoint(const txPoint& p)
//        : x(p.x), y(p.y), z(p.z) {}
//
//    txPoint& operator=(const txPoint& p)
//    {
//        x = p.x, y = p.y, z = p.z;
//        return *this;
//    }
//
//    bool operator==(const txPoint& p)
//    {
//        return fabs(x - p.x) < 1e-7 && fabs(y - p.y) < 1e-7 && \
//            fabs(z - p.z) < 1e-7;
//    }
//};

TX_NAMESPACE_CLOSE(hadmap)
