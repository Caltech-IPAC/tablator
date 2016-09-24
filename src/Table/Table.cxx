#include "../Table.hxx"
#include "../data_size.hxx"

const std::string tablator::Table::null_bitfield_flags_description
    = "Packed bit array indicating whether an entry is null";

tablator::Table::Table (const std::vector<Column> &Columns,
                        const std::map<std::string, std::string> &property_map)
{
  const size_t null_flags_size = (Columns.size () + 7) / 8;
  append_column ("null_bitfield_flags", Data_Type::UINT8_LE, null_flags_size,
                 Field_Properties (null_bitfield_flags_description));

  for (auto &c : Columns)
    { append_column (c); }

  for (auto &p : property_map)
    { properties.emplace_back (p.first, Property (p.second)); }
}

tablator::Table::Table (const boost::filesystem::path &input_path)
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
  else if (format.is_csv () || format.is_tsv ())
    {
      read_dsv (input_path,format);
    }
  else
    {
      throw std::runtime_error ("Unsupported input format: "
                                + input_path.string ());
    }
}
