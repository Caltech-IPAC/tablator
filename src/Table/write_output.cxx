#include "../Table.hxx"

void tablator::Table::write_output (const boost::filesystem::path &path,
                                    const Format &format)
{
  const bool use_stdout (path.string () == "-");
  if (format.is_fits ())
    {
      write_fits (path);
    }
  else if (format.is_hdf5 ())
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

      switch (format.enum_format)
        {
        case Format::Enums::JSON:
        case Format::Enums::JSON5:
          boost::property_tree::write_json (out, generate_property_tree(true), true);
          break;
        case Format::Enums::VOTABLE:
          boost::property_tree::write_xml
            (out, generate_property_tree(false),
             boost::property_tree::xml_writer_make_settings(' ',2));
          break;
        case Format::Enums::CSV:
          write_csv_tsv (out, ',');
          break;
        case Format::Enums::TSV:
          write_csv_tsv (out, '\t');
          break;
        case Format::Enums::IPAC_TABLE:
        case Format::Enums::TEXT:
          write_ipac_table (out);
          break;
        case Format::Enums::HTML:
          write_html (out);
          break;
        case Format::Enums::UNKNOWN:
        default:
          throw std::runtime_error ("Unknown output type");
          break;
        }
    }
}
