#pragma once

#include <stdexcept>
#include <string>
#include "Data_Type.hxx"

namespace tablator {
inline Data_Type string_to_Data_Type(const std::string &s) {
    if (s == "INT8_LE") {
        return Data_Type::INT8_LE;
    }
    if (s == "UINT8_LE") {
        return Data_Type::UINT8_LE;
    }
    if (s == "INT16_LE") {
        return Data_Type::INT16_LE;
    }
    if (s == "UINT16_LE") {
        return Data_Type::UINT16_LE;
    }
    if (s == "INT32_LE") {
        return Data_Type::INT32_LE;
    }
    if (s == "UINT32_LE") {
        return Data_Type::UINT32_LE;
    }
    if (s == "INT64_LE") {
        return Data_Type::INT64_LE;
    }
    if (s == "UINT64_LE") {
        return Data_Type::UINT64_LE;
    }
    if (s == "FLOAT32_LE") {
        return Data_Type::FLOAT32_LE;
    }
    if (s == "FLOAT64_LE") {
        return Data_Type::FLOAT64_LE;
    }
    if (s == "STRING") {
        return Data_Type::CHAR;
    }
    throw std::runtime_error("Unknown Data_Type string: " + s);
}
}  // namespace tablator
