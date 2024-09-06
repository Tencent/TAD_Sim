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
#include "fbm/impl/data_2d.hpp"
#include <functional>

namespace fbm {
namespace impl {
//! Simple class that can take an qualifier for which indicies to ignore.
//!
//! Excellent for Brownian bridge where we do not want to set the boundaries
class Data2DOut {
public:
    Data2DOut(Data2D& data, std::function<bool(int, int)> acceptor) :
        data(data), acceptor(acceptor) {
        // empty
    }

    double& operator()(int x, int y) {

        if (acceptor(x, y)) {
            return data(x, y);
        } else {
            return throw_away;
        }
    }
private:
    double throw_away = 42.0;
    Data2D data;
    std::function<bool(int, int)> acceptor;
};

}
}
