#include "../Table.hxx"

const std::string tablator::Table::null_bitfield_flags_description
    = "Packed bit array indicating whether an entry is null";

tablator::Table::Table (const std::vector<Column> &columns,
                        const std::map<std::string, std::string> &property_map)
    : compound_type (size_t (1))
{
  const size_t null_flags_size = (columns.size () + 7) / 8;
  append_array_column ("null_bitfield_flags", H5::PredType::STD_U8LE,
                       null_flags_size);

  fields_properties[0].description = null_bitfield_flags_description;
  for (auto &c : columns)
    {
      auto type = c.second.first.first;
      const size_t member_size = c.second.first.first.getSize ()
                                 * c.second.first.second;
      compound_type.setSize (compound_type.getSize () + member_size);
      append_column (c.first, type, c.second.first.second);
      *fields_properties.rbegin () = c.second.second;
    }

  for (auto &p : property_map)
    properties.emplace_back (p.first, Property (p.second));
}

tablator::Table::Table (const boost::filesystem::path &input_path)
  : compound_type (size_t (1))
{
  Format format (input_path);
  if (format.is_hdf5 ())
    {
      read_hdf5 (input_path);
    }
  else if (format.is_fits ())
    {
      read_fits (input_path);
    }
  else if (format.is_ipac_table ())
    {
      read_ipac_table (input_path);
    }
  else if (format.is_json5 ())
    {
      read_json5 (input_path);
    }
  else if (format.is_votable () || format.is_json ())
    {
      boost::property_tree::ptree tree;
      boost::filesystem::ifstream file (input_path);
      if (format.is_votable ())
        boost::property_tree::read_xml (file, tree);
      else
        boost::property_tree::read_json (file, tree);
      read_property_tree_as_votable (tree);
    }
  else
    {
      throw std::runtime_error ("Unsupported input format: "
                                + input_path.string ());
    }
}
