#include "../../Table.hxx"

std::string tablator::Table::to_ipac_string (const Data_Type &type) const
{
  /// Write out unsigned integers as integers for backwards compatibility
  switch (type)
    {
    case Data_Type::INT8_LE:
    case Data_Type::UINT8_LE:
    case Data_Type::INT16_LE:
    case Data_Type::UINT16_LE:
    case Data_Type::INT32_LE:
      return "int";
  /// Unsigned 32 bit ints do not fit in ints, so we use a long.
    case Data_Type::UINT32_LE:
    case Data_Type::INT64_LE:
    case Data_Type::UINT64_LE:
      return "long";
    case Data_Type::FLOAT32_LE:
      return "float";
    case Data_Type::FLOAT64_LE:
      return "double";
    case Data_Type::CHAR:
      return "char";
    default:
      throw std::runtime_error (
          "Unexpected HDF5 data type in tablator::Table::to_ipac_string");
    }
}
