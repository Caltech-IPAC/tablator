#pragma once

#include "to_string.hxx"

namespace tablator
{
inline size_t data_size (const Data_Type &data_type)
{
  switch (data_type)
    {
    case Data_Type::INT8_LE:
    case Data_Type::UINT8_LE:
      return 8;
    case Data_Type::INT16_LE:
    case Data_Type::UINT16_LE:
      return 16;
    case Data_Type::INT32_LE:
    case Data_Type::UINT32_LE:
    case Data_Type::FLOAT32_LE:
      return 32;
    case Data_Type::INT64_LE:
    case Data_Type::UINT64_LE:
    case Data_Type::FLOAT64_LE:
      return 64;
    default:
      throw std::runtime_error ("Invalid value for data_size (): "
                                + to_string (data_type));
    }
}
}
