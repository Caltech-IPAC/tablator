#include "H5_to_Data_Type.hxx"

#include <stdexcept>

namespace tablator
{
Data_Type H5_to_Data_Type (const H5::DataType &H5_type)
{
  if (H5_type == H5::PredType::STD_I8LE)
    {
      return Data_Type::INT8_LE;
    }
  else if (H5_type == H5::PredType::STD_U8LE)
    {
      return Data_Type::UINT8_LE;
    }
  else if (H5_type == H5::PredType::STD_I16LE)
    {
      return Data_Type::INT16_LE;
    }
  else if (H5_type == H5::PredType::STD_U16LE)
    {
      return Data_Type::UINT16_LE;
    }
  else if (H5_type == H5::PredType::STD_I32LE)
    {
      return Data_Type::INT32_LE;
    }
  else if (H5_type == H5::PredType::STD_U32LE)
    {
      return Data_Type::UINT32_LE;
    }
  else if (H5_type == H5::PredType::STD_I64LE)
    {
      return Data_Type::INT64_LE;
    }
  else if (H5_type == H5::PredType::STD_U64LE)
    {
      return Data_Type::UINT64_LE;
    }
  else if (H5_type == H5::PredType::IEEE_F32LE)
    {
      return Data_Type::FLOAT32_LE;
    }
  else if (H5_type == H5::PredType::IEEE_F64LE)
    {
      return Data_Type::FLOAT64_LE;
    }
  else if (H5_type.getClass () == H5T_STRING)
    {
      return Data_Type::STRING;
    }
  else if (H5_type.getClass () == H5T_ARRAY)
    {
      return H5_to_Data_Type (H5_type.getSuper ());
    }
  else
    {
      throw std::runtime_error ("Unknown H5::DataType");
    }
}
}
