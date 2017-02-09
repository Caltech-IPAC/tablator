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

tablator::Table::Table (const boost::filesystem::path &input_path,
                        const Format &format)
{
  // FIXME: This has too many if(){} else {} clauses
  if (format.is_hdf5 ())
    {
      H5::Exception::dontPrint ();
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
  else if (format.is_votable ())
    {
      read_votable (input_path);
    }
  else if (format.is_json ())
    {
      read_json (input_path);
    }
  else if (format.is_csv () || format.is_tsv ())
    {
      read_dsv (input_path,format);
    }
  else
    {
      bool is_read_successful (false);
      try
        {
          H5::Exception::dontPrint ();
          read_hdf5 (input_path);
          is_read_successful=true;
        }
      catch (...)
        {}
      if (!is_read_successful)
        {
          try
            {
              read_fits (input_path);
              is_read_successful=true;
            }
          catch (...)
            {}
        }
      if (!is_read_successful)
        {
          try
            {
              read_ipac_table (input_path);
              is_read_successful=true;
            }
          catch (...)
            {}
        }
      if (!is_read_successful)
        {
          try
            {
              read_json5 (input_path);
              is_read_successful=true;
            }
          catch (...)
            {}
        }
      if (!is_read_successful)
        {
          try
            {
              read_votable(input_path);
              is_read_successful=true;
            }
          catch (...)
            {}
        }
      if (!is_read_successful)
        {
          try
            {
              read_json(input_path);
              is_read_successful=true;
            }
          catch (...)
            {}
        }
      if (!is_read_successful)
        {
          try
            {
              read_dsv (input_path,Format("csv"));
              is_read_successful=true;
            }
          catch (...)
            {}
        }
      if (!is_read_successful)
        {
          try
            {
              read_dsv (input_path,Format("tsv"));
              is_read_successful=true;
            }
          catch (...)
            {}
        }
      if (!is_read_successful)
        throw std::runtime_error ("Unsupported input: " + input_path.string ()
                                  + ". Tried all formats.");
    }
}
