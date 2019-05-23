#pragma once

#include <iostream>

#include "Data_Type.hxx"

namespace tablator {

void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                         const size_t &array_size, const uint8_t *data,
                         const char &separator, const Data_Type &alt_datatype);

inline void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                                const size_t &array_size, const uint8_t *data,
                                const char &separator) {
    write_type_as_ascii(os, type, array_size, data, separator, type);
}

inline void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                                const size_t &array_size, const uint8_t *data) {
    write_type_as_ascii(os, type, array_size, data, ' ', type);
}

}  // namespace tablator
