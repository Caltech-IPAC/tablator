#include "../../Table.hxx"

std::string tablator::Table::to_ipac_string (const H5::DataType &type) const
{
  /// Write out unsigned integers as integers for backwards compatibility 
  if (type==H5::PredType::STD_I8LE || type==H5::PredType::STD_U8LE
      || type==H5::PredType::STD_I16LE || type==H5::PredType::STD_U16LE
      || type==H5::PredType::STD_I32LE)
    {
      return "int";
    }
  /// Unsigned 32 bit ints do not fit in ints, so we use a long.
  else if (type==H5::PredType::STD_U32LE || type==H5::PredType::STD_I64LE
           || type==H5::PredType::STD_U64LE)
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
  else if (type.getClass ()==H5T_STRING)
    {
      return "char";
    }
  else if (type.getClass ()==H5T_ARRAY)
    {
      /// FIXME: Do something reasonable for array output
      throw std::runtime_error ("Array types are unsupported in IPAC Tables");
    }
  else
    {
      throw std::runtime_error
        ("Unexpected HDF5 data type in tablator::Table::to_ipac_string");
    }
}
