#pragma once

#include "Data_Type_ostream.hxx"

#include <string>
#include <stdexcept>

namespace tablator
{
inline std::string to_xml_string (const Data_Type &type)
{
  switch (type)
    {
    case Data_Type::INT8_LE:
      return "boolean";
      break;
    case Data_Type::UINT8_LE:
      return "unsignedByte";
      break;
    case Data_Type::INT16_LE:
      return "short";
      break;
    case Data_Type::UINT16_LE:
      return "ushort";
      break;
    case Data_Type::INT32_LE:
      return "int";
      break;
    case Data_Type::UINT32_LE:
      return "uint";
      break;
    case Data_Type::INT64_LE:
      return "long";
      break;
    case Data_Type::UINT64_LE:
      return "ulong";
      break;
    case Data_Type::FLOAT32_LE:
      return "float";
      break;
    case Data_Type::FLOAT64_LE:
      return "double";
      break;
    case Data_Type::CHAR:
      return "char";
      break;
    default:
      throw std::runtime_error ("Unexpected data type in "
                                "tablator::to_xml_string: "
                                + std::to_string(static_cast<int>(type)));
      break;
    }
}
}
