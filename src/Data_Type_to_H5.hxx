#pragma once

#include "Data_Type.hxx"

#include <H5Cpp.h>

#include <stdexcept>

namespace tablator {
inline H5::DataType Data_Type_to_H5(const Data_Type &type) {
    switch (type) {
        case Data_Type::INT8_LE:
            return H5::PredType::STD_I8LE;
        case Data_Type::UINT8_LE:
            return H5::PredType::STD_U8LE;
        case Data_Type::INT16_LE:
            return H5::PredType::STD_I16LE;
        case Data_Type::UINT16_LE:
            return H5::PredType::STD_U16LE;
        case Data_Type::INT32_LE:
            return H5::PredType::STD_I32LE;
        case Data_Type::UINT32_LE:
            return H5::PredType::STD_U32LE;
        case Data_Type::INT64_LE:
            return H5::PredType::STD_I64LE;
        case Data_Type::UINT64_LE:
            return H5::PredType::STD_U64LE;
        case Data_Type::FLOAT32_LE:
            return H5::PredType::IEEE_F32LE;
        case Data_Type::FLOAT64_LE:
            return H5::PredType::IEEE_F64LE;
        case Data_Type::CHAR:
            return H5::PredType::C_S1;
        default:
            throw std::runtime_error("Unknown Data_Type: " +
                                     std::to_string(static_cast<int>(type)));
    }
}
}  // namespace tablator
