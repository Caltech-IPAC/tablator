#include "../Table.hxx"

tablator::Table::Table (const std::vector<Column> &columns,
                        const std::map<std::string, std::string> &property_map)
  : compound_type (size_t(1)), offsets({0})
{
  row_size=0;
  const size_t null_flags_size=(columns.size ()+7)/8;
  string_types.emplace_back (0, null_flags_size);
  append_member ("null_bitfield_flags", *string_types.rbegin ());

  fields_properties.push_back (Field_Properties("Packed bit array indicating "
                                                "whether an entry is null", {}));
  types.push_back (H5::PredType::C_S1);

  for (auto &c : columns)
    {
      auto type = c.second.first.first;
      const size_t member_size=
        c.second.first.first.getSize () * c.second.first.second;
      compound_type.setSize (compound_type.getSize () + member_size);
      if (type == H5::PredType::C_S1)
        {
          string_types.emplace_back (0, c.second.first.second);
          append_member (c.first, *string_types.rbegin ());
        }
      // else if (c.second.first.second!=1)
      //   {
      //     array_types.emplace_back (type, 1, c.second.first.second);
      //     append_member (c.first, *array_types.rbegin ());
      //   }
      else
        {
          append_member (c.first, type);
        }
      fields_properties.emplace_back (c.second.second);
      types.push_back (type);
    }

  for (auto &p : property_map)
    properties.emplace_back (p.first, Property (p.second));
}
