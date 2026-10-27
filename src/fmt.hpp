#pragma once

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

inline std::string format_number(double x) {
    if (std::isnan(x)) return "nan";
    if (std::isinf(x)) return x > 0 ? "inf" : "-inf";

    std::ostringstream os;
    os << std::setprecision(12) << x;
    std::string s = os.str();

    if (s.find('e') != std::string::npos || s.find('E') != std::string::npos) {
        return s;
    }
    if (s.find('.') != std::string::npos) {
        while (!s.empty() && s.back() == '0') s.pop_back();
        if (!s.empty() && s.back() == '.') s.pop_back();
    }
    return s.empty() ? "0" : s;
}
