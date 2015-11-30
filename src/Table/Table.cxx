#include "../Table.hxx"

const std::string tablator::Table::null_bitfield_flags_description
= "Packed bit array indicating whether an entry is null";

tablator::Table::Table (const std::vector<Column> &columns,
                        const std::map<std::string, std::string> &property_map)
  : compound_type (size_t(1))
{
  const size_t null_flags_size=(columns.size ()+7)/8;
  append_array_member ("null_bitfield_flags", H5::PredType::STD_U8LE, null_flags_size);

  fields_properties[0].description = null_bitfield_flags_description;
  for (auto &c : columns)
    {
      auto type = c.second.first.first;
      const size_t member_size=
        c.second.first.first.getSize () * c.second.first.second;
      compound_type.setSize (compound_type.getSize () + member_size);
      if (type == H5::PredType::C_S1)
        {
          append_string_member (c.first, c.second.first.second);
        }
      else if (c.second.first.second!=1)
        {
          append_array_member (c.first, type, c.second.first.second);
        }
      else
        {
          append_member (c.first, type);
        }
      *fields_properties.rbegin ()=c.second.second;
    }

  for (auto &p : property_map)
    properties.emplace_back (p.first, Property (p.second));
}
