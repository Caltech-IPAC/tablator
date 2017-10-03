#pragma once

#include "Data_Type.hxx"

#include <stdexcept>
#include <string>

namespace tablator
{
inline std::string Data_Type_to_SQLite (const Data_Type &type,
                                        const size_t &array_size)
{
  if (type != Data_Type::CHAR && array_size != 1)
    {
      throw std::runtime_error ("Arrays not supported in SQLite");
    }
  switch (type)
    {
    case Data_Type::INT8_LE:
    case Data_Type::UINT8_LE:
    case Data_Type::INT16_LE:
    case Data_Type::UINT16_LE:
    case Data_Type::INT32_LE:
    case Data_Type::UINT32_LE:
    case Data_Type::INT64_LE:
    case Data_Type::UINT64_LE:
      return "INTEGER";
    case Data_Type::FLOAT32_LE:
    case Data_Type::FLOAT64_LE:
      return "REAL";
    case Data_Type::CHAR:
      return "TEXT";
    default:
      throw std::runtime_error ("Unknown Data_Type: "
                                + std::to_string (static_cast<int>(type)));
    }
}
}
