#include "../Table.hxx"

TAP::Table::Table (
    const std::vector<std::pair<std::string,
                                std::pair<std::pair<H5::PredType, size_t>,
                                          Field_Properties> > > &columns,
    const std::map<std::string, std::string> &property_map)
    : compound_type (std::accumulate (
          columns.begin (), columns.end (), static_cast<size_t>(0),
          [](const size_t &sum,
             const std::pair<std::string,
                             std::pair<std::pair<H5::PredType, size_t>,
                                       Field_Properties> > &c)
          {
            return sum + c.second.first.first.getSize ()
                         * c.second.first.second;
          })),
      row_size (compound_type.getSize ())
{
  size_t offset{ 0 };
  for (auto &c : columns)
    {
      auto type = c.second.first.first;
      if (type == H5::PredType::NATIVE_CHAR)
        {
          string_types.emplace_back (H5::PredType::NATIVE_CHAR,
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
        throw TAP::Error (500, "Unknown HDF5 type in compound_type with id: "
                               + std::to_string (type.getId ()));
    }
  offsets.push_back (offset);

  for (auto &p : property_map)
    properties.insert (std::make_pair (p.first, Property (p.second)));
}
