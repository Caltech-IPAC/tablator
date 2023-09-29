#pragma once

#include <stdexcept>

#include "Data_Type.hxx"
#include "to_string.hxx"

namespace tablator {
inline size_t data_size(const Data_Type &data_type) {
    switch (data_type) {
        case Data_Type::INT8_LE:
        case Data_Type::UINT8_LE:
            return 1;
        case Data_Type::INT16_LE:
        case Data_Type::UINT16_LE:
            return 2;
        case Data_Type::INT32_LE:
        case Data_Type::UINT32_LE:
        case Data_Type::FLOAT32_LE:
            return 4;
        case Data_Type::INT64_LE:
        case Data_Type::UINT64_LE:
        case Data_Type::FLOAT64_LE:
            return 8;
        case Data_Type::CHAR:
            return 1;
        default:
            throw std::runtime_error("Invalid value for data_size (): " +
                                     to_string(data_type));
    }
}
}  // namespace tablator
