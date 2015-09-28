#include "../Table.hxx"

tablator::Table::Table (const std::vector<Column> &columns,
                        const std::map<std::string, std::string> &property_map)
  : compound_type (size_t(1)), offsets({0})
{
  row_size=0;
  const size_t null_flags_size=(columns.size ()+7)/8;
  string_types.emplace_back (0, null_flags_size);
  append_member ("null_bitfield_flags", *string_types.rbegin ());

  fields_properties.push_back (Field_Properties("Packed bit array indicating whether an entry is null", {}));
  types.push_back (Type::STRING);

  for (auto &c : columns)
    {
      auto type = c.second.first.first;
      const size_t member_size=
        c.second.first.first.getSize () * c.second.first.second;
      compound_type.setSize (compound_type.getSize () + member_size);
      if (type == H5::PredType::NATIVE_CHAR)
        {
          string_types.emplace_back (0, c.second.first.second);
          append_member (c.first, *string_types.rbegin ());
        }
      else
        {
          append_member (c.first, type);
        }
      fields_properties.emplace_back (c.second.second);

      if (type == H5::PredType::NATIVE_UCHAR)
        types.push_back (Type::BOOLEAN);
      else if (type == H5::PredType::NATIVE_INT16)
        types.push_back (Type::SHORT);
      else if (type == H5::PredType::NATIVE_INT32)
        types.push_back (Type::INT);
      else if (type == H5::PredType::NATIVE_INT64)
        types.push_back (Type::LONG);
      else if (type == H5::PredType::NATIVE_FLOAT)
        types.push_back (Type::FLOAT);
      else if (type == H5::PredType::NATIVE_DOUBLE)
        types.push_back (Type::DOUBLE);
      else if (type == H5::PredType::NATIVE_CHAR)
        types.push_back (Type::STRING);
      else
        throw std::runtime_error (
            "Unknown HDF5 type in compound_type with id: "
            + std::to_string (type.getId ()));
    }

  for (auto &p : property_map)
    properties.emplace_back (p.first, Property (p.second));
}
