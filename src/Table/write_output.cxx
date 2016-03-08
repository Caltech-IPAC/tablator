#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

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
        case Format::Enums::VOTABLE:
          {
            std::string tabledata_string (boost::uuids::to_string
                                          (boost::uuids::random_generator()()));
            boost::property_tree::ptree tree (generate_property_tree
                                              (tabledata_string));
            std::stringstream ss;
            if (format.enum_format!=Format::Enums::VOTABLE)
              {
                boost::property_tree::write_json (ss, tree, true);
              }
            else
              {
                boost::property_tree::write_xml
                  (ss, tree,
                   boost::property_tree::xml_writer_make_settings(' ',2));
              }
            std::string s (ss.str ());
            size_t tabledata_offset (s.find (tabledata_string));
            bool is_json (format.enum_format!=Format::Enums::VOTABLE);
            out << s.substr (0, tabledata_offset - (is_json ? 2 : 0));
            write_tabledata (out, is_json);
            out << s.substr (tabledata_offset+tabledata_string.size ()
                             + (is_json ? 2 : 0));
          }
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
