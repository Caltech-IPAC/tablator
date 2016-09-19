#include <iomanip>
#include <inttypes.h>
#include <limits>
#include <cmath>

#include "../../Table.hxx"
#include "../write_type_as_ascii.hxx"

void tablator::Table::write_ipac_table (std::ostream &os) const
{
  std::vector<size_t> ipac_column_widths = get_column_width ();

  write_ipac_table_header (os);
  int total_record_width = 0;

  os << "|";
  os << std::right;
  for (size_t i = 1; i < columns.size (); ++i)
    {
      total_record_width += (ipac_column_widths[i] + 1);
      os << std::setw (ipac_column_widths[i])
         << columns[i].name << "|";
    }
  os << "\n|";

  for (size_t i = 1; i < columns.size (); ++i)
    {
      if (columns[i].type != Data_Type::CHAR && columns[i].array_size != 1)
        { throw std::runtime_error ("Column '" + columns[i].name +
                                    "' is an array which is unsupported in "
                                    "IPAC Tables"); }
      os << std::setw (ipac_column_widths[i])
         << to_ipac_string (columns[i].type)
         << "|";
    }

  os << "\n|";

  for (size_t i = 1; i < columns.size (); ++i)
    {
      os << std::setw (ipac_column_widths[i]);
      auto unit = columns[i].field_properties.attributes.find ("unit");
      if (unit == columns[i].field_properties.attributes.end ())
        {
          os << " ";
        }
      else
        {
          os << unit->second;
        }
      os << "|";
    }
  os << "\n|";

  for (size_t i = 1; i < columns.size (); ++i)
    {
      os << std::setw (ipac_column_widths[i]);
      auto null = columns[i].field_properties.values.null;
      if (null.empty ())
        {
          os << "null";
        }
      else
        {
          os << null;
        }
      os << "|";
    }
  os << "\n";

  std::stringstream ss;
  for (size_t row_offset = 0; row_offset < data.size ();
       row_offset += row_size ())
    {
      /// Skip the null bitfield flag
      for (size_t i = 1; i < columns.size (); ++i)
        {
          ss << " " << std::setw (ipac_column_widths[i]);
          size_t offset = offsets[i] + row_offset;
          if (is_null (row_offset, i))
            {
              auto null_value = columns[i].field_properties.values.null;
              if (null_value.empty ())
                ss << "null";
              else
                ss << null_value;
            }
          else
            {
              auto data_type = columns[i].type;
              /// Silently convert unsigned 64 bit ints to signed 64
              /// bit ints, since ipac tables do not support unsigned
              /// 64 bit ints.
              if (data_type == Data_Type::UINT64_LE)
                data_type = Data_Type::INT64_LE;
              /// Do some gymnastics because we want to write a byte
              /// as an int
              if (data_type == Data_Type::UINT8_LE)
                {
                  ss << static_cast<const uint16_t>(
                            static_cast<uint8_t>(*(data.data () + offset)));
                }
              else
                {
                  std::stringstream ss_temp;
                  write_type_as_ascii (ss_temp, data_type, columns[i].array_size,
                                       data.data () + offset, output_precision);
                  std::string s(ss_temp.str());
                  /// Turn newlines into spaces
                  auto newline_location (s.find('\n'));
                  while (newline_location != std::string::npos)
                    {
                      s[newline_location]=' ';
                      newline_location = s.find('\n',newline_location+1);
                    }
                  ss << s;
                }
            }
        }
      ss << " \n";
      os << ss.str ();
      ss.str ("");
    }
}
