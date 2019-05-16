#pragma once

#include <H5Cpp.h>

namespace tablator {
class HDF5_Property {
public:
    const char *name;
    const char *value;
    hvl_t attributes;

    HDF5_Property(const char *Name, const char *Value, const hvl_t &Attributes)
            : name(Name), value(Value), attributes(Attributes) {}
};
}  // namespace tablator
