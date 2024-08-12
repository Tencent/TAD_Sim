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

#include <tinyxml2.h>
#include <fstream>
#include <string>

#include "functions.h"
#include "XSDParser.h"
#include "CodeGenerator.h"
#include "xsd2cpp.h"

namespace xsd2cpp {

void xsd2cpp(const std::string &filename, std::ostream &cppPublicHeaderStream, std::ostream &cppHeaderStream,
             std::ostream &cppSourceStream, const std::string &headerFilename, const xsd2cpp::Config &config) {
  // read header
  std::ifstream t(headerFilename);
  std::string header((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

  // replace content
  replace_string(header, "$year", config.year);
  replace_string(header, "$date", config.date);
  replace_string(header, "$owner", config.owner);
  replace_string(header, "$email", config.email);

  // xml document
  tinyxml2::XMLDocument xml_doc;
  tinyxml2::XMLError eResult = xml_doc.LoadFile(filename.c_str());
  if (eResult != tinyxml2::XML_SUCCESS) throw std::runtime_error("File could not be loaded!");

  // get root element
  auto schema = xml_doc.FirstChildElement("xsd:schema");
  if (schema == nullptr) throw std::runtime_error("Schema element could not be found!");

  // parse scheme
  XSDParser::parseScheme(schema);

  // generate code
  CodeGenerator::generate(cppPublicHeaderStream, cppHeaderStream, cppSourceStream, config.libKey, config.libName,
                          header);
}

}  // namespace xsd2cpp
