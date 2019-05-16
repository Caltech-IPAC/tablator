#pragma once

#include "HDF5_Field_Properties.hxx"

namespace tablator {
class HDF5_Column {
public:
    const char *name, *type;
    uint64_t array_size;
    HDF5_Field_Properties field_properties;
    HDF5_Column(const char *Name, const char *Type, const uint64_t &Array_size,
                const HDF5_Field_Properties &Field_properties)
            : name(Name),
              type(Type),
              array_size(Array_size),
              field_properties(Field_properties) {}
};
}  // namespace tablator
