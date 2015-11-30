#include "../../../Table.hxx"

void
tablator::Table::append_ipac_data_member (const std::string &name,
                                          const std::string &data_type,
                                          const size_t &num_elements)
{
  /// We use "string".compare (0,size,t) because it is valid to
  /// abbreviate the type name
  std::string t=boost::to_lower_copy(data_type);
  const size_t string_size (t.size ());
  if (std::string ("boolean").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_I8LE);
    }
  else if (std::string ("byte").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_U8LE);
    }
  else if (std::string ("short").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_I16LE);
    }
  else if (std::string ("ushort").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_U16LE);
    }
  else if (std::string ("int").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_I32LE);
    }
  else if (std::string ("uint").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_U32LE);
    }
  else if (std::string ("long").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_I64LE);
    }
  else if (std::string ("ulong").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::STD_U64LE);
    }
  else if (std::string ("float").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::IEEE_F32LE);
    }
  else if (std::string ("double").compare (0, string_size, t)==0
           || std::string ("real").compare (0, string_size, t)==0)
    {
      append_member (name, H5::PredType::IEEE_F64LE);
    }
  else if (std::string ("char").compare (0, string_size, t)==0
           || std::string ("date").compare (0, string_size, t)==0)
    {
      append_string_member (name, num_elements);
    }
  else
    {
      throw std::runtime_error ("Unknown data type in IPAC table: "
                                + data_type);
    }
}
