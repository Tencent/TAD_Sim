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
#include <cmath>
#include <cassert>
#include <iostream>
namespace fbm {
namespace impl {
class VarianceFBM2D {
public:

    VarianceFBM2D(const double* X, int nx, double H, bool set_sides_to_zero = true)
        : set_sides_to_zero(set_sides_to_zero), X(X), nx(nx), ny(nx), H(H) {
        assert(0 < H);
        assert(H < 1);

    }

    //! @todo Figure out the global indexing here at some point
    double operator()(int x, int y, int level) const {
        if (set_sides_to_zero) {
            // If we are on the boundary, we don't return any variance
            if (x == 0 || x == nx - 1 || y == 0 || y == ny - 1) {
                return 0;
            }

            assert(counter >= 0 && counter < (nx - 2) * (nx - 2));
        } else {
            assert(counter >= 0 && counter < (nx - 1) * (nx - 1));
        }





        return variancefBm(level) * X[counter++];
    }

    double variancefBm(int level) const {
        return std::sqrt(
                (1.0 - std::pow(2.0, (2 * H - 2))) / (std::pow(2, (2 * level * H))));
    }

private:
    const bool set_sides_to_zero;
    const double* X;
    const int nx, ny;
    const double H;

    mutable long long counter = 0;
};
}
}
