#pragma once

#include "HDF5_Min_Max.hxx"

namespace tablator {
class HDF5_Values {
public:
    HDF5_Min_Max min, max;
    const char *ID, *type, *null, *ref;
    hvl_t options;
};
}  // namespace tablator
