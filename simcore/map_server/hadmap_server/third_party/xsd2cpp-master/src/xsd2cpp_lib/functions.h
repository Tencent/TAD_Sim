//
// Created by Jens Klimke on 2019-04-14.
//

#ifndef XSD2CPP_FUNCTIONS_H
#define XSD2CPP_FUNCTIONS_H

#include <string>
#include <sstream>
#include <vector>


/**
 * String formatter
 * @tparam Args
 * @param format Format string
 * @param args Inputs
 * @return Formatted string
 */
template<typename ... Args>
std::string string_format(const std::string &format, Args ... args) {
    auto size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    char *buf(new char[size]);
    snprintf(buf, size, format.c_str(), args ...);
    return std::string(buf, buf + size - 1); // We don't want the '\0' inside
}


/**
 * Replaces a search string (from) within a string (str) by a string (to)
 * @param str String to be searched in
 * @param from String to be replaced
 * @param to String to replace
 * @return
 */
inline bool replace_string(std::string &str, const std::string &from, const std::string &to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}


#endif //XSD2CPP_FUNCTIONS_H
