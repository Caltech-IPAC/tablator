#pragma once

#include "Data_Type.hxx"
#include <iostream>

namespace tablator
{
inline std::ostream &operator<< (std::ostream &os,
                                 const Data_Type &type)
{
  switch (type)
    {
    case Data_Type::INT8_LE:
      os << "INT8_LE";
      break;
    case Data_Type::UINT8_LE:
      os << "UINT8_LE";
      break;
    case Data_Type::INT16_LE:
      os << "INT16_LE";
      break;
    case Data_Type::UINT16_LE:
      os << "UINT16_LE";
      break;
    case Data_Type::INT32_LE:
      os << "INT32_LE";
      break;
    case Data_Type::UINT32_LE:
      os << "UINT32_LE";
      break;
    case Data_Type::INT64_LE:
      os << "INT64_LE";
      break;
    case Data_Type::UINT64_LE:
      os << "UINT64_LE";
      break;
    case Data_Type::FLOAT32_LE:
      os << "FLOAT32_LE";
      break;
    case Data_Type::FLOAT64_LE:
      os << "FLOAT64_LE";
      break;
    case Data_Type::CHAR:
      os << "STRING";
      break;
    }
  return os;
}
}
