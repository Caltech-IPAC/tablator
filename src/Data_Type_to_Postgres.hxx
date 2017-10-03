#pragma once

#include "Data_Type.hxx"

#include <stdexcept>
#include <string>

namespace tablator
{
inline std::string Data_Type_to_Postgres (const Data_Type &type,
                                          const size_t &array_size)
{
  std::string result;
  switch (type)
    {
    case Data_Type::INT8_LE:
    case Data_Type::UINT8_LE:
    case Data_Type::INT16_LE:
      result = "SMALLINT";
      break;
    case Data_Type::UINT16_LE:
    case Data_Type::INT32_LE:
      result = "INTEGER";
      break;
    case Data_Type::UINT32_LE:
    case Data_Type::INT64_LE:
      result = "BIGINT";
      break;
    case Data_Type::UINT64_LE:
      result = "NUMERIC(19)";
      break;
    case Data_Type::FLOAT32_LE:
      result = "REAL";
      break;
    case Data_Type::FLOAT64_LE:
      result = "DOUBLE PRECISION";
      break;
    case Data_Type::CHAR:
      result = "VARCHAR(" + std::to_string (array_size) + ")";
      break;
    default:
      throw std::runtime_error ("Unknown Data_Type: "
                                + std::to_string (static_cast<int>(type)));
      break;
    }
  if (type != Data_Type::CHAR && array_size != 1)
    {
      result += " ARRAY[" + std::to_string (array_size) + "]";
    }
  return result;
}
}
