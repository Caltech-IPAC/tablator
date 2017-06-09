#pragma once

#include "Data_Type.hxx"

#include <stdexcept>

namespace tablator
{
inline std::string Data_Type_to_Oracle (const Data_Type &type)
{
  switch (type)
    {
    case Data_Type::INT8_LE:
    case Data_Type::UINT8_LE:
      return "NUMBER(3)";
    case Data_Type::INT16_LE:
    case Data_Type::UINT16_LE:
      return "NUMBER(5)";
    case Data_Type::INT32_LE:
    case Data_Type::UINT32_LE:
      return "NUMBER(10)";
    case Data_Type::INT64_LE:
    case Data_Type::UINT64_LE:
      return "NUMBER(19)";
    case Data_Type::FLOAT32_LE:
      return "BINARY_FLOAT";
    case Data_Type::FLOAT64_LE:
      return "BINARY_DOUBLE";
    case Data_Type::CHAR:
      return "VARCHAR2";
    default:
      throw std::runtime_error ("Unknown Data_Type: "
                                + std::to_string (static_cast<int>(type)));
    }
}
}
