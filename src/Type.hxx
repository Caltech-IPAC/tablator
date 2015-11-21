#pragma once

#include <string>
#include <stdexcept>

namespace tablator
{
/// Type names are mostly lifted directly from the IVOA TAP spec.
/// IVOA has fixed length char[] arrays.  We just use a string.
enum class Type : char
{ BOOLEAN,
  SHORT,
  INT,
  LONG,
  FLOAT,
  DOUBLE,
  STRING };

inline std::string to_string (const tablator::Type &type)
{
  std::string result;
  switch (type)
    {
    case tablator::Type::BOOLEAN:
      result = "boolean";
      break;
    case tablator::Type::SHORT:
      result = "short";
      break;
    case tablator::Type::INT:
      result = "int";
      break;
    case tablator::Type::LONG:
      result = "long";
      break;
    case tablator::Type::FLOAT:
      result = "float";
      break;
    case tablator::Type::DOUBLE:
      result = "double";
      break;
    case tablator::Type::STRING:
      result = "char";
      break;
    default:
      throw std::runtime_error (
          "Unexpected data type in Field_Properties_to_xml: "
          + std::to_string (static_cast<int>(type)));
    }
  return result;
}
}

