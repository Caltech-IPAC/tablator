#pragma once

namespace tablator
{
inline std::ostream &operator<< (std::ostream &os, tablator::Data_Type &type)
{
  switch (type)
    {
    case INT8_LE:
      os << "INT8_LE";
      break;
    case UINT8_LE:
      os << "UINT8_LE";
      break;
    case INT16_LE:
      os << "INT16_LE";
      break;
    case UINT16_LE:
      os << "UINT16_LE";
      break;
    case INT32_LE:
      os << "INT32_LE";
      break;
    case UINT32_LE:
      os << "UINT32_LE";
      break;
    case INT64_LE:
      os << "INT64_LE";
      break;
    case UINT64_LE:
      os << "UINT64_LE";
      break;
    case FLOAT32_LE:
      os << "FLOAT32_LE";
      break;
    case FLOAT64_LE:
      os << "FLOAT64_LE";
      break;
    case STRING:
      os << "STRING";
      break;
    case ARRAY:
      os << "ARRAY";
      break;
    }
  return os;
}
}
