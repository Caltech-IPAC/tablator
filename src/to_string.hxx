#pragma once

#include <sstream>
#include "Data_Type_ostream.hxx"

namespace tablator {
inline std::string to_string(const Data_Type &type) {
    std::stringstream ss;
    ss << type;
    return ss.str();
}
}  // namespace tablator
