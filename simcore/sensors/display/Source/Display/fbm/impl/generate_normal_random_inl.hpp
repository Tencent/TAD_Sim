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
#include <random>
#include <exception>
#include <sstream>

#define FBM_ASSERT_NON_NEGATIVE(x) { \
    if(x<0) {\
        std::stringstream message; \
        message << "The can generate for a non negativ number of elements.\n"\
                << "Given: " << x << " (variable "<< #x <<")\n\n"\
<< "In " << __FILE__ << " at " << __LINE__ << "\n"; \
    throw std::runtime_error(message.str().c_str());\
} \
}
namespace fbm {
inline std::vector<double> generate_normal_random(int number_of_elements) {
    FBM_ASSERT_NON_NEGATIVE(number_of_elements);
    std::vector<double> random_numbers(size_t(number_of_elements), 0);

    std::default_random_engine generator;
    std::uniform_real_distribution<double> distribution(0.0, 1.0);

    for (auto& d : random_numbers) {
        d = distribution(generator);
    }

    return random_numbers;
}
}

#undef FBM_ASSERT_GREATER_THAN_ZERO
