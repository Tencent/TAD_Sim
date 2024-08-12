// Copyright (c) 2020 Jens Klimke (jens.klimke@rwth-aachen.de). All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// Created by Jens Klimke on 2020-08-14.
//


#ifndef PARSE_XSD_XSD2CPP_H
#define PARSE_XSD_XSD2CPP_H

#include <ostream>

namespace xsd2cpp {

    struct Config {
        std::string libKey;
        std::string libName;
        std::string year;
        std::string date;
        std::string owner;
        std::string email;
    };

    /**
     * Parses the given xsd file and generates c++ code
     * @param filename The xsd input file
     * @param cppPublicHeaderStream The output c++ header file stream
     * @param cppHeaderStream The output c++ header file stream
     * @param cppSourceStream The output c++ source file stream
     * @param headerFilename The header comment definition file
     * @param libKey The output library key (code compliant key for namespaces etc.)
     * @param libName The output library name (the library name used for file naming)
     */
    void xsd2cpp(const std::string &filename, std::ostream &cppPublicHeaderStream, std::ostream &cppHeaderStream,
                 std::ostream &cppSourceStream, const std::string &headerFilename, const Config &config);

}

#endif //PARSE_XSD_XSD2CPP_H
