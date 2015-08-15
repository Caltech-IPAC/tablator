#include "../Table.hxx"

tablator::Table::Table (const std::vector<Column> &columns,
                        const std::map<std::string, std::string> &property_map)
    : compound_type ([&](){
        size_t sum=(columns.size ()+7)/8;
        for (auto &c: columns)
          sum+=c.second.first.first.getSize () * c.second.first.second;
        return sum;
      }()),
      row_size (compound_type.getSize ())
{
  size_t offset(0);
  const size_t null_flags_size=(columns.size ()+7)/8;

  string_types.emplace_back (0,
                             null_flags_size);
  compound_type.insertMember ("null_bitfield_flags", offset,
                              *string_types.rbegin ());
  offsets.push_back (offset);
  offset+=null_flags_size;
  fields_properties.push_back (Field_Properties("Packed bit array indicating whether an entry is null", {}));
  types.push_back (Type::STRING);

  for (auto &c : columns)
    {
      auto type = c.second.first.first;
      if (type == H5::PredType::NATIVE_CHAR)
        {
          string_types.emplace_back (0,
                                     c.second.first.second);
          compound_type.insertMember (c.first, offset,
                                      *string_types.rbegin ());
        }
      else
        {
          compound_type.insertMember (c.first, offset, type);
        }
      offsets.push_back (offset);
      offset += c.second.first.first.getSize () * c.second.first.second;
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
  offsets.push_back (offset);

  for (auto &p : property_map)
    properties.insert (std::make_pair (p.first, Property (p.second)));
}
