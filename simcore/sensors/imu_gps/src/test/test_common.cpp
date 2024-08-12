/**
 * @file TestCommon.cpp
 * @author kekesong ()
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "test_common.h"

namespace test {
namespace math {
double Distance3d(const Eigen::Vector3d &pt1, const Eigen::Vector3d &pt2) { return (pt2 - pt1).norm(); }
}  // namespace math
}  // namespace test
