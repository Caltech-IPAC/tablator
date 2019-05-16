#pragma once

#include <string>

namespace tablator {
class Min_Max {
public:
    std::string value;
    bool inclusive;

    Min_Max() : inclusive(true) {}
    Min_Max(const std::string &Value, const bool &Inclusive)
            : value(Value), inclusive(Inclusive) {}
    bool empty() const { return value.empty(); }
};
}  // namespace tablator
