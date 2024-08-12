// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCore>
#include "tx_marco.h"
#include "tx_type_def.h"

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
using txAffine = Eigen::Affine3d;
using txAxis = Eigen::Vector3d;
using txAngleAxis = Eigen::AngleAxisd;
using txTranslation = Eigen::Translation3d;
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
using txAffine = Eigen::Affine3f;
using txAxis = Eigen::Vector3f;
using txAngleAxis = Eigen::AngleAxisf;
using txTranslation = Eigen::Translation3f;
#endif

TX_NAMESPACE_CLOSE(Base)
