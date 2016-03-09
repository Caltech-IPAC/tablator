#pragma once

#include <string>
#include <stdexcept>
#include <H5Cpp.h>

namespace tablator
{
inline std::string to_string (const H5::DataType &type)
{
  if (type == H5::PredType::STD_I8LE)
    {
      return "boolean";
    }
  else if (type == H5::PredType::STD_U8LE)
    {
      return "unsignedByte";
    }
  else if (type == H5::PredType::STD_I16LE)
    {
      return "short";
    }
  else if (type == H5::PredType::STD_U16LE)
    {
      return "ushort";
    }
  else if (type == H5::PredType::STD_I32LE)
    {
      return "int";
    }
  else if (type == H5::PredType::STD_U32LE)
    {
      return "uint";
    }
  else if (type == H5::PredType::STD_I64LE)
    {
      return "long";
    }
  else if (type == H5::PredType::STD_U64LE)
    {
      return "ulong";
    }
  else if (type == H5::PredType::IEEE_F32LE)
    {
      return "float";
    }
  else if (type == H5::PredType::IEEE_F64LE)
    {
      return "double";
    }
  else if (type.getClass () == H5T_STRING)
    {
      return "char";
    }
  else if (type.getClass () == H5T_ARRAY)
    {
      return "array<" + to_string (type.getSuper ()) + ">";
    }
  else
    {
      throw std::runtime_error (
          "Unexpected HDF5 data type in tablator::to_string: "
          + type.fromClass ());
    }
}
}
