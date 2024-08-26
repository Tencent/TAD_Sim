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
#include <vector>
#include <iostream>
namespace fbm {
namespace impl {
class VarianceFBM1D {
public:
    VarianceFBM1D(const std::vector<double>& X, int nx, double H)
        : X(X.data()), nx(nx), H(H) {}

    VarianceFBM1D(const double* X, int nx, double H)
        : X(X), nx(nx), H(H) {}

    double operator()(int x, int level) const {
        // If we are on the boundary, we don't return any variance
        if (x == 0 || x == nx - 1) {
            return 0;
        }


        // Todo: make this pure bitshift one day.
        const long long index = (1l << (level - 1)) + x / (long(nx - 1) >>
                (level - 1));

        assert(index > 0 && index < nx - 1);
        return variancefBm(level) * X[index - 1];
    }

    double variancefBm(int level) const {
        return std::sqrt(
                (1.0 - std::pow(2.0, (2 * H - 2))) / (std::pow(2, (2 * level * H))));
    }

private:
    const double* X;
    int nx;
    double H;
};
}
}
