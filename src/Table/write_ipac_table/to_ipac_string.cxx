#include "../../Table.hxx"

std::string tablator::Table::to_ipac_string (const H5::PredType &type) const
{
  if (type==H5::PredType::STD_I8LE || type==H5::PredType::STD_I16LE
      || type==H5::PredType::STD_I32LE)
    {
      return "int";
    }
  else if (type==H5::PredType::STD_I64LE)
    {
      return "long";
    }
  else if (type==H5::PredType::IEEE_F32LE || type==H5::PredType::IEEE_F64LE)
    {
      return "double";
    }
  else if (type==H5::PredType::C_S1)
    {
      return "char";
    }
  else
    {
      throw std::runtime_error
        ("Unexpected HDF5 data type in tablator::Table::to_ipac_string");
    }
}
