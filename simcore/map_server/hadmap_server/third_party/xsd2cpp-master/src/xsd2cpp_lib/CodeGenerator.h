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
// Created by Jens Klimke on 2020-08-11
//

#include <ostream>
#include <exception>
#include <cmath>
#include <algorithm>
#include <memory>
#include <string>

#include "DataType.h"
#include "functions.h"

#ifndef XSD2CPP_CODE_GENERATOR_H
#define XSD2CPP_CODE_GENERATOR_H

namespace xsd2cpp {


    class CodeGenerator {

    public:

        static void generate(std::ostream &fsp,
                             std::ostream &fsh,
                             std::ostream &fss,
                             const std::string &libKey,
                             const std::string &libName,
                             const std::string &header) {


            // PUBLIC HEADER FILE

            std::string inclGuard = libKey + "_STRUCTURE_H";
            std::transform(inclGuard.begin(), inclGuard.end(), inclGuard.begin(), ::toupper);

            fsp << string_format(header, (libKey + ".h").c_str()) << std::endl << std::endl;
            fsp << "#ifndef " << inclGuard << "\n";
            fsp << "#define " << inclGuard << std::endl << std::endl;
            fsp << "#include <string>" << std::endl;
            fsp << "#include <memory>" << std::endl;
            fsp << "#include <vector>" << std::endl << std::endl;
            fsp << "namespace " << libKey << " {" << std::endl << std::endl;


            // create attribute class
            fsp << "\tnamespace xsd {" << std::endl
                << "\ttemplate<typename T>" << std::endl
                << "\tstruct Attribute : public std::shared_ptr<T> {" << std::endl
                << "\t\tAttribute() = default;" << std::endl
                << "\t\tvirtual ~Attribute() = default;" << std::endl
                << "\t\tAttribute<T> &operator=(const T &v) {" << std::endl
                << "\t\t\tthis->reset(new T(v));" << std::endl
                << "\t\t\treturn *this;" << std::endl
                << "\t\t}" << std::endl << std::endl
                << "\t\tAttribute<T> &create() {" << std::endl
                << "\t\t\tthis->reset(new T);" << std::endl
                << "\t\t\treturn *this;" << std::endl
                << "\t\t}" << std::endl
                << "\t};" << std::endl << std::endl
                << "\ttypedef Attribute<double> d_double;" << std::endl
                << "\ttypedef Attribute<int> d_int;" << std::endl
                << "\ttypedef Attribute<unsigned int> d_uint;" << std::endl
                << "\ttypedef Attribute<std::string> d_string;" << std::endl
                << "\ttypedef Attribute<float> d_float;" << std::endl
                << "\ttemplate<typename T>" << std::endl
                << "\tusing Vector = std::vector<T>;" << std::endl
                << "\t} // namespace xsd" << std::endl << std::endl;

            // iterate over data types to generate typedefs and pre-defs
            for (const auto &t : DataType::dataTypesVector) {

                auto d = t->typeDefString();
                if (!d.empty())
                    fsp << "\t" << d << std::endl;

            }


            fsp << std::endl << std::endl;

            // iterate over data types to generate full structure
            for (const auto &t : DataType::dataTypesVector) {

                auto d = t->typeDefStringFull();
                if (!d.empty())
                    fsp << d << std::endl << std::endl;

            }


            fsp << std::endl;
            fsp << "} // namespace " << libKey << std::endl << std::endl;
            fsp << "#endif // " << inclGuard << std::endl;



            // HEADER FILE

            inclGuard = "XML_PARSER_" + libKey + "_hpp";
            std::transform(inclGuard.begin(), inclGuard.end(), inclGuard.begin(), ::toupper);

            fsh << string_format(header, (libKey + ".hpp").c_str()) << std::endl << std::endl;
            fsh << "#ifndef " << inclGuard << "\n";
            fsh << "#define " << inclGuard << std::endl << std::endl;
            fsh << "#include <vector>" << std::endl;
            fsh << "#include <string>" << std::endl;
            fsh << "#include <tinyxml2.h>" << std::endl << std::endl;
            fsh << "#include \"" << libName << ".h\"" << std::endl << std::endl;
            fsh << "namespace " << libKey << " {" << std::endl << std::endl;


            // iterate over data types to generate parser function definitions
            for (const auto &t : DataType::dataTypesVector) {

                auto tp = t->parseHeader();
                if (!tp.empty())
                    fsh << "\t" << tp << ";" << std::endl;

            }
            // iterate over data types to generate save function definitions
            for (const auto &t : DataType::dataTypesVector) {

                auto tp = t->saveHeader();
                if (!tp.empty())
                    fsh << "\t" << tp << ";" << std::endl;

            }


            fsh << std::endl;
            fsh << "} // namespace " << libKey << std::endl << std::endl;
            fsh << "#endif // " << inclGuard << std::endl;




            // SOURCE FILE

            fss << string_format(header, (libKey + ".cc").c_str()) << std::endl << std::endl;
            fss << "#include \"" << libName << ".hpp\"" << std::endl << std::endl;
            fss << "namespace " << libKey << " {" << std::endl << std::endl;


            // iterate over data types to generate parser function implementations
            for (const auto &t : DataType::dataTypesVector) {

                auto tp = t->parseHeader();
                auto tb = t->parseBody();

                if (!tp.empty())
                    fss << string_format("%s {\n\n%s\n\n\treturn true;\n\n}", tp.c_str(), tb.c_str()) << std::endl
                        << std::endl;

            }
            // iterate over data types to generate save function implementations
            for (const auto &t : DataType::dataTypesVector) {

                auto tp = t->saveHeader();
                auto tb = t->saveBody();

                if (!tp.empty())
                    fss << string_format("%s {\n\n%s\n\n\treturn true;\n\n}", tp.c_str(), tb.c_str()) << std::endl
                        << std::endl;

            }


            fss << std::endl;
            fss << "} // namespace " << libKey << std::endl;

        }

    };

}

#endif //XSD2CPP_CODE_GENERATOR_H
