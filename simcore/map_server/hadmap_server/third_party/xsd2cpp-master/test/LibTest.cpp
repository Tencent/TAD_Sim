// Copyright (c) 2020 Jens Klimke <jens.klimke@rwth-aachen.de>
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
// Created by Jens Klimke on 2020-08-12.
//

#include <gtest/gtest.h>
#include <xsd2cpp.h>

#ifndef RES_DIR
#define RES_DIR "./"
#endif

TEST(LibTest, OpenDRIVE_1_5) {
  using namespace xsd2cpp;

  // create file name
  std::string filename(RES_DIR);
  filename += "/OpenDRIVE_1.5M.xsd";

  // create header filename
  std::string headerFilename(RES_DIR);
  headerFilename += "/header.txt";

  // lib name and version
  std::string libName = "OpenDRIVE";
  std::string libVersion = "odr1_5";

  // out streams
  std::stringstream fsp;
  std::stringstream fsh;
  std::stringstream fss;

  // run lib
  xsd2cpp::xsd2cpp(filename, fsp, fsh, fss, headerFilename, Config());

  // check
  std::cout << fsp.str() << std::endl << std::endl << std::endl << std::endl;
  std::cout << fsp.str() << std::endl << std::endl << std::endl << std::endl;
  std::cout << fsp.str() << std::endl << std::endl << std::endl << std::endl;
}

TEST(LibTest, OpenSCENARIO_1_0) {
  using namespace xsd2cpp;

  // create file name
  std::string filename(RES_DIR);
  filename += "/OpenSCENARIO_1.0.xsd";

  // create header filename
  std::string headerFilename(RES_DIR);
  headerFilename += "/header.txt";

  // lib name and version
  std::string libName = "OpenSCENARIO";
  std::string libVersion = "osc1_0";

  // out streams
  std::stringstream fsp;
  std::stringstream fsh;
  std::stringstream fss;

  // run lib
  xsd2cpp::xsd2cpp(filename, fsp, fsh, fss, headerFilename, Config());

  // check
  std::cout << fsp.str() << std::endl << std::endl << std::endl << std::endl;
  std::cout << fsp.str() << std::endl << std::endl << std::endl << std::endl;
  std::cout << fsp.str() << std::endl << std::endl << std::endl << std::endl;
}
