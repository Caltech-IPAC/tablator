#include "../Table.hxx"

void tablator::Table::write_output (const boost::filesystem::path &path,
                                    const Format &format)
{
  const bool use_stdout (path.string () == "-");
  if (format.index->first == Format::enum_format::FITS)
    {
      write_fits (path);
    }
  else if (format.index->first == Format::enum_format::HDF5)
    {
      if (use_stdout)
        write_hdf5 (std::cout);
      else
        write_hdf5 (path);
    }
  else
    {
      boost::filesystem::ofstream file_output;
      if (!use_stdout)
        file_output.open (path);
      std::ostream &out (use_stdout ? std::cout : file_output);

      switch (format.index->first)
        {
        case Format::enum_format::JSON:
        case Format::enum_format::JSON5:
          boost::property_tree::write_json (out, generate_property_tree(), true);
          break;
        case Format::enum_format::VOTABLE:
          boost::property_tree::write_xml
            (out, generate_property_tree(),
             boost::property_tree::xml_writer_make_settings(' ',2));
          break;
        case Format::enum_format::CSV:
          write_csv_tsv (out, ',');
          break;
        case Format::enum_format::TSV:
          write_csv_tsv (out, '\t');
          break;
        case Format::enum_format::IPAC_TABLE:
        case Format::enum_format::TEXT:
          write_ipac_table (out);
          break;
        case Format::enum_format::HTML:
          write_html (out);
          break;
        case Format::enum_format::UNKNOWN:
        default:
          throw std::runtime_error ("Unknown output type");
          break;
        }
    }
}
