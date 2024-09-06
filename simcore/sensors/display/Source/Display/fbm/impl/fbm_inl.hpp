/* Copyright (c) 2019 Kjetil Olsen Lye, ETH Zurich
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include "fbm/impl/data_3d.hpp"
#include "fbm/impl/data_3d_out.hpp"
#include "fbm/impl/variance_fbm_3d.hpp"

#include "fbm/impl/data_2d.hpp"
#include "fbm/impl/data_2d_out.hpp"
#include "fbm/impl/variance_fbm_2d.hpp"

#include "fbm/impl/data_1d.hpp"
#include "fbm/impl/data_1d_out.hpp"
#include "fbm/impl/variance_fbm_1d.hpp"

#include <exception>
#include "fbm/impl/is_power_of_two.hpp"

#include <string>
#include <sstream>

#define FBM_ASSERT_POWER_OF_TWO(x) { \
    if(!fbm::impl::is_power_of_two(x) && x!= 1) {\
        std::stringstream message; \
        message << "The fBm bridge algorithm needs the size to be a power of two.\n"\
                << "Given: " << x << " (variable "<< #x <<")\n\n"\
<< "In " << __FILE__ << " at " << __LINE__ << "\n"; \
    throw std::runtime_error(message.str().c_str());\
} \
}

#define FBM_ASSERT_GREATER_THAN_ZERO(x) { \
    if(x<=0) {\
        std::stringstream message; \
        message << "The fBm bridge algorithm needs the size positive.\n"\
                << "Given: " << x << " (variable "<< #x <<")\n\n"\
<< "In " << __FILE__ << " at " << __LINE__ << "\n"; \
    throw std::runtime_error(message.str().c_str());\
} \
}

namespace fbm {

inline std::vector<double> fractional_brownian_bridge_3d(double H, int nx,
    const std::vector<double>& X, bool set_sides_to_zero) {
    FBM_ASSERT_GREATER_THAN_ZERO(nx)

    std::vector<double> bridge((nx + 1) * (nx + 1) * (nx + 1), 0);

    fractional_brownian_bridge_3d(bridge.data(), H, nx, X.data(),
        set_sides_to_zero);

    return bridge;
}

inline void fractional_brownian_bridge_3d(double* data_out_pointer, double H,
    int nx, const double* X, bool set_sides_to_zero) {

    FBM_ASSERT_POWER_OF_TWO(nx)
    FBM_ASSERT_GREATER_THAN_ZERO(nx)
    impl::Data3D data(data_out_pointer, nx + 1);

    impl::VarianceFBM3D variance(X, nx + 1, H, set_sides_to_zero);

    int level_nx = 1;

    int level = 1;

    // We do not want to set data on the boundary
    auto not_on_boundary = [&](int x, int y, int z) -> bool {

        if (set_sides_to_zero) {
            return x != 0 && x != nx && y != 0 && y != nx && z != 0 && z != nx;
        } else {
            return true;
        }
    };

    // This throws away data we are trying to set on the boundary
    impl::Data3DOut data_out(data, not_on_boundary);

    while (level_nx < nx) {

        int factor = nx / (2 * level_nx);

        for (int z = 0; z < level_nx; ++z) {
            for (int y = 0; y < level_nx; ++y) {
                for (int x = 0; x < level_nx; ++x) {

                    int mid_x = factor * (2 * x + 1);
                    int left_x = factor * 2 * x;
                    int right_x = factor * 2 * (x + 1);

                    int mid_y = factor * (2 * y + 1);
                    int left_y = factor * 2 * y;
                    int right_y = factor * 2 * (y + 1);

                    int mid_z = factor * (2 * z + 1);
                    int left_z = factor * 2 * z;
                    int right_z = factor * 2 * (z + 1);

                    // LINE SEGMENTS
                    // x
                    data_out(mid_x, left_y, left_z) =
                        (data(left_x, left_y, left_z) + data(right_x, left_y, left_z)) /
                        2.0 +
                        variance(mid_x, left_y, left_z, level);

                    // y
                    data_out(left_x, mid_y, left_z) =
                        (data(left_x, left_y, left_z) + data(left_x, right_y, left_z)) /
                        2.0 +
                        variance(left_x, mid_y, left_z, level);

                    // z
                    data_out(left_x, left_y, mid_z) =
                        (data(left_x, left_y, left_z) + data(left_x, left_y, right_z)) /
                        2.0 +
                        variance(left_x, left_y, mid_z, level);

                    // SIDES
                    // x
                    data_out(mid_x, mid_y, left_z) =
                        (data(left_x, left_y, left_z) + data(right_x, left_y, left_z) +
                            data(left_x, right_y, left_z) + data(right_x, right_y, left_z)) /
                        4.0 +
                        variance(mid_x, mid_y, left_z, level);

                    // y
                    data_out(mid_x, left_y, mid_z) =
                        (data(left_x, left_y, left_z) + data(right_x, left_y, left_z) +
                            data(left_x, left_y, right_z) + data(right_x, left_y, right_z)) /
                        4.0 +
                        variance(mid_x, left_y, mid_z, level);

                    // z
                    data_out(left_x, mid_y, mid_z) =
                        (data(left_x, left_y, left_z) + data(left_x, right_y, left_z) +
                            data(left_x, left_y, right_z) + data(left_x, right_y, right_z)) /
                        4.0 +
                        variance(left_x, mid_y, mid_z, level);

                    // CENTER POINT
                    data_out(mid_x, mid_y, mid_z) =
                        (data(left_x, left_y, left_z) + data(right_x, left_y, left_z) +
                            data(right_x, right_y, left_z) +
                            data(right_x, right_y, right_z) +
                            data(right_x, left_y, right_z) + data(left_x, right_y, left_z) +
                            data(left_x, right_y, right_z) + data(left_x, left_y, right_z)) /
                        8.0 +
                        variance(mid_x, mid_y, mid_z, level);
                }
            }
        }

        level_nx *= 2;

        level += 1;
    }
}

inline std::vector<double> fractional_brownian_bridge_2d(double H, int nx,
    const std::vector<double>& X, bool set_sides_to_zero) {
    FBM_ASSERT_GREATER_THAN_ZERO(nx)

    std::vector<double> bridge((nx + 1) * (nx + 1), 0);

    fractional_brownian_bridge_2d(bridge.data(), H, nx, X.data(),
        set_sides_to_zero);

    return bridge;
}

inline void fractional_brownian_bridge_2d(double* data_out_pointer, double H,
    int nx, const double* X, bool set_sides_to_zero) {
    FBM_ASSERT_POWER_OF_TWO(nx)
    FBM_ASSERT_GREATER_THAN_ZERO(nx)


    impl::Data2D data(data_out_pointer, nx + 1);

    impl::VarianceFBM2D variance(X, nx + 1, H, set_sides_to_zero);

    int level_nx = 1;

    int level = 1;

    // We do not want to set data on the boundary
    auto not_on_boundary = [&](int x, int y) -> bool {
        if (set_sides_to_zero) {
            return x != 0 && x != nx && y != 0 && y != nx;
        } else {
            return true;
        }
    };

    // This throws away data we are trying to set on the boundary
    impl::Data2DOut data_out(data, not_on_boundary);

    while (level_nx < nx) {

        const int factor = nx / (2 * level_nx);

        for (int y = 0; y < level_nx; ++y) {
            for (int x = 0; x < level_nx; ++x) {

                int mid_x = factor * (2 * x + 1);
                int left_x = factor * 2 * x;
                int right_x = factor * 2 * (x + 1);

                int mid_y = factor * (2 * y + 1);
                int left_y = factor * 2 * y;
                int right_y = factor * 2 * (y + 1);

                // LINE SEGMENTS
                // x
                data_out(mid_x, left_y) =
                    (data(left_x, left_y) + data(right_x, left_y)) / 2.0 +
                    variance(mid_x, left_y, level);

                // y
                data_out(left_x, mid_y) =
                    (data(left_x, left_y) + data(left_x, right_y)) / 2.0 +
                    variance(left_x, mid_y, level);

                // SIDE
                // x
                data_out(mid_x, mid_y) =
                    (data(left_x, left_y) + data(right_x, left_y) +
                        data(left_x, right_y) + data(right_x, right_y)) /
                    4.0 +
                    variance(mid_x, mid_y, level);
            }
        }

        level_nx *= 2;

        level += 1;
    }
}


inline std::vector<double> fractional_brownian_bridge_1d(double H, int nx,
    const std::vector<double>& X) {
    FBM_ASSERT_GREATER_THAN_ZERO(nx)

    std::vector<double> bridge((nx + 1), 0);

    fractional_brownian_bridge_1d(bridge.data(), H, nx, X.data());

    return bridge;
}

inline void fractional_brownian_bridge_1d(double* data_out_pointer, double H,
    int nx, const double* X) {
    FBM_ASSERT_POWER_OF_TWO(nx)
    FBM_ASSERT_GREATER_THAN_ZERO(nx)
    impl::Data1D data(data_out_pointer, nx + 1);

    impl::VarianceFBM1D variance(X, nx + 1, H);

    int level_nx = 1;

    int level = 1;

    // We do not want to set data on the boundary
    auto not_on_boundary = [&](int x) {
        return x != 0 && x != nx;
    };

    // This throws away data we are trying to set on the boundary
    impl::Data1DOut data_out(data, not_on_boundary);

    while (level_nx < nx) {

        int factor = nx / (2 * level_nx);

        for (int x = 0; x < level_nx; ++x) {

            int mid_x = factor * (2 * x + 1);
            int left_x = factor * 2 * x;
            int right_x = factor * 2 * (x + 1);

            // LINE SEGMENTS
            // x
            data_out(mid_x) =
                (data(left_x) + data(right_x)) / 2.0 + variance(mid_x, level);
        }

        level_nx *= 2;

        level += 1;
    }
}


inline std::vector<double> fractional_brownian_motion_1d(double H, int nx,
    const std::vector<double>& X) {
    FBM_ASSERT_GREATER_THAN_ZERO(nx);

    std::vector<double> motion((nx + 1), 0);

    fractional_brownian_motion_1d(motion.data(), H, nx, X.data());

    return motion;

}

inline void fractional_brownian_motion_1d(double* data_out, double H, int nx,
    const double* X) {

    // Fix the last element, the rest will just be plain old Brownian motion
    data_out[nx] = X[0];
    fractional_brownian_bridge_1d(data_out, H, nx, X + 1);
}
} // namespace fbm

#undef FBM_ASSERT_GREATER_THAN_ZERO
#undef FBM_ASSERT_POWER_OF_TWO
