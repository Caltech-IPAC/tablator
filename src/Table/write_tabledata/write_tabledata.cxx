#include <iomanip>
#include <limits>
#include <cmath>

#include "../../Table.hxx"
#include "../../to_string.hxx"
#include "../../write_type_as_ascii.hxx"

namespace tablator
{
std::string decode_links (const std::string &encoded);

void Table::write_tabledata (std::ostream &os,
                             const Format::Enums &output_format) const
{
  std::string tr_prefix, tr_suffix, td_prefix, td_suffix;
  std::string tabledata_indent = "                    ";
  const bool is_json (output_format == Format::Enums::JSON
                      || output_format == Format::Enums::JSON5);
  if (is_json)
    {
      tabledata_indent = "                    ";
      std::string tr_indent = tabledata_indent + "    ";
      tr_prefix = tr_indent + "[\n";
      tr_suffix = tr_indent + "]";

      std::string td_indent = tr_indent + "    ";
      td_prefix = td_indent + "\"";
      td_suffix = "\"";

      os << '\n' << tabledata_indent << "[\n";
    }
  else
    {
      tabledata_indent = "        ";
      std::string tr_indent = tabledata_indent + "  ";
      tr_prefix = tr_indent + "<TR>\n";
      tr_suffix = tr_indent + "</TR>";

      std::string td_indent = tr_indent + "  ";
      td_prefix = td_indent + "<TD>";
      td_suffix = "</TD>";
      os << '\n';
    }

  for (size_t row_offset = 0; row_offset < data.size ();
       row_offset += row_size ())
    {
      os << tr_prefix;

      /// Skip the null bitfield flag
      for (size_t column = 1; column < columns.size (); ++column)
        {
          std::stringstream td;
          if (!is_null (row_offset, column))
            {
              write_type_as_ascii (td, columns[column].type,
                                   columns[column].array_size,
                                   data.data () + row_offset + offsets[column],
                                   output_precision);
            }
          os << td_prefix;
          switch (output_format)
            {
            case Format::Enums::JSON:
            case Format::Enums::JSON5:
              // FIXME: This uses the undocumented character escapes.
              os << boost::property_tree::json_parser::create_escapes (
                  td.str ());
              break;
            case Format::Enums::HTML:
              {
                os << decode_links (
                    boost::property_tree::xml_parser::encode_char_entities (
                        td.str ()));
              }
              break;
            default:
              os << boost::property_tree::xml_parser::encode_char_entities (
                  td.str ());
              break;
            }
          os << td_suffix;
          if (is_json && column < columns.size () - 1)
            {
              os << ',';
            }
          os << '\n';
        }
      os << tr_suffix;
      if (is_json && row_offset < data.size () - row_size ())
        {
          os << ',';
        }
      os << '\n';
    }
  os << tabledata_indent;
  if (is_json)
    {
      os << "]\n";
    }
}
}
