#pragma once

#include <string>
#include <stdexcept>

namespace tablator
{
inline std::string to_string (const H5::DataType &type)
{
  if (type==H5::PredType::STD_U8LE)
    {
      return "byte";
    }
  else if (type==H5::PredType::STD_I8LE)
    {
      return "boolean";
    }
  else if (type==H5::PredType::STD_I16LE)
    {
      return "short";
    }
  else if (type==H5::PredType::STD_I32LE)
    {
      return "int";
    }
  else if (type==H5::PredType::STD_I64LE)
    {
      return "long";
    }
  else if (type==H5::PredType::IEEE_F32LE)
    {
      return "float";
    }
  else if (type==H5::PredType::IEEE_F64LE)
    {
      return "double";
    }
  else if (type==H5::PredType::C_S1)
    {
      return "char";
    }
  // FIXME: This does not handle arrays
  else
    {
      throw std::runtime_error
        ("Unexpected HDF5 data type in tablator::to_string: "
         + type.fromClass ());
    }
}
}

