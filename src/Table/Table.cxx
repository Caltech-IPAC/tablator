#include "../Table.hxx"

const std::string tablator::Table::null_bitfield_flags_name
    = "null_bitfield_flags";
const std::string tablator::Table::null_bitfield_flags_description
    = "Packed bit array indicating whether an entry is null";

tablator::Table::Table (const std::vector<Column> &Columns,
                        const std::map<std::string, std::string> &property_map)
{
  if (Columns.empty ())
    {
      throw std::runtime_error ("This table has no columns");
    }
  const size_t null_flags_size = (Columns.size () + 7) / 8;
  append_column (null_bitfield_flags_name, Data_Type::UINT8_LE,
                 null_flags_size,
                 Field_Properties (null_bitfield_flags_description));

  for (auto &c : Columns)
    {
      append_column (c);
    }

  for (auto &p : property_map)
    {
      properties.emplace_back (p.first, Property (p.second));
    }
}

tablator::Table::Table (const boost::filesystem::path &input_path,
                        const Format &format)
{
  switch (format.enum_format)
    {
    case Format::Enums::HDF5:
      H5::Exception::dontPrint ();
      read_hdf5 (input_path);
      break;
    case Format::Enums::FITS:
      read_fits (input_path);
      break;
    case Format::Enums::IPAC_TABLE:
    case Format::Enums::TEXT:
      read_ipac_table (input_path);
      break;
    case Format::Enums::JSON5:
      read_json5 (input_path);
      break;
    case Format::Enums::VOTABLE:
      read_votable (input_path);
      break;
    case Format::Enums::JSON:
      read_json (input_path);
      break;
    case Format::Enums::CSV:
    case Format::Enums::TSV:
      read_dsv (input_path, format);
      break;
    case Format::Enums::UNKNOWN:
      read_unknown (input_path);
      break;
    default:
      throw std::runtime_error ("Unsupported input format: '"
                                + format.string ()
                                + "' for input file: " + input_path.string ());
      break;
    }
  if (columns.size () < 2)
    {
      throw std::runtime_error ("This file has no columns: "
                                + input_path.string ());
    }
}  

tablator::Table::Table (std::istream &input_stream,
                        const Format &format)
{
  switch (format.enum_format)
    {
      // FIXME: Implement streaming for HDF5 and FITS
    // case Format::Enums::HDF5:
    //   H5::Exception::dontPrint ();
    //   read_hdf5 (input_stream);
    //   break;
    // case Format::Enums::FITS:
    //   read_fits (input_stream);
    //   break;
    case Format::Enums::IPAC_TABLE:
    case Format::Enums::TEXT:
      read_ipac_table (input_stream);
      break;
    case Format::Enums::JSON5:
      read_json5 (input_stream);
      break;
    case Format::Enums::VOTABLE:
      read_votable (input_stream);
      break;
    case Format::Enums::JSON:
      read_json (input_stream);
      break;
    case Format::Enums::CSV:
    case Format::Enums::TSV:
      read_dsv (input_stream, format);
      break;
    case Format::Enums::UNKNOWN:
      read_unknown (input_stream);
      break;
    default:
      throw std::runtime_error ("Unsupported input format for streaming: "
                                + format.string ());
      break;
    }
  if (columns.size () < 2)
    {
      throw std::runtime_error ("This stream has no columns");
    }
}  
