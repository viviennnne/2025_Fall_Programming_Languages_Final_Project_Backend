#ifndef SIMPLE_JSON_HPP
#define SIMPLE_JSON_HPP

#include <string>
#include <sstream>

namespace SimpleJson {

inline std::string escape(const std::string& s) {
    std::ostringstream os;
    for (char c : s) {
        if (c == '\"') os << "\\\"";
        else if (c == '\\') os << "\\\\";
        else os << c;
    }
    return os.str();
}

inline std::string makeMessage(const std::string& status,
                               const std::string& message) {
    std::ostringstream os;
    os << "{"
       << "\"status\":\"" << escape(status) << "\","
       << "\"message\":\"" << escape(message) << "\""
       << "}";
    return os.str();
}

} // namespace SimpleJson

#endif
