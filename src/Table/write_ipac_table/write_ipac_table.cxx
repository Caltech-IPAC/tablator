#include <iomanip>
#include <inttypes.h>
#include <limits>
#include <cmath>

#include "../../Table.hxx"
#include "../write_type_as_ascii.hxx"

void tablator::Table::write_ipac_table (std::ostream &os) const
{
  std::vector<size_t> ipac_column_widths = get_column_width ();

  const size_t num_members = compound_type.getNmembers () - 1;
  write_ipac_table_header (os, num_members);
  int total_record_width = 0;

  os << "|";
  os << std::right;
  for (size_t i = 0; i < num_members; ++i)
    {
      total_record_width += (ipac_column_widths[i + 1] + 1);
      os << std::setw (ipac_column_widths[i + 1])
         << compound_type.getMemberName (i + 1) << "|";
    }
  os << "\n|";

  for (size_t i = 0; i < num_members; ++i)
    {
      os << std::setw (ipac_column_widths[i + 1]);
      os << to_ipac_string (compound_type.getMemberDataType (i + 1));
      os << "|";
    }

  if (fields_properties.size () > 0)
    os << "\n|";
  else
    os << "\n";

  size_t i = 0;
  for (auto &f : fields_properties)
    {
      if (i == 0)
        {
          ++i;
          continue;
        }
      os << std::setw (ipac_column_widths[i]);
      auto unit = f.attributes.find ("unit");
      if (unit == f.attributes.end ())
        {
          os << " ";
        }
      else
        {
          os << unit->second;
        }
      os << "|";
      ++i;
    }
  if (fields_properties.size () > 1)
    os << "\n|";

  i = 0;
  for (auto &f : fields_properties)
    {
      if (i == 0)
        {
          ++i;
          continue;
        }
      os << std::setw (ipac_column_widths[i]);
      auto null = f.values.null;
      if (null.empty ())
        {
          os << "null";
        }
      else
        {
          os << null;
        }
      os << "|";
      ++i;
    }
  if (fields_properties.size () > 1)
    os << "\n";

  std::stringstream ss;
  for (size_t row_offset = 0; row_offset < data.size ();
       row_offset += row_size)
    {
      /// Skip the null bitfield flag
      for (size_t column = 0; column < num_members; ++column)
        {
          ss << " " << std::setw (ipac_column_widths[column + 1]);
          size_t offset = offsets[column + 1] + row_offset;
          if (is_null (row_offset, column + 1))
            {
              auto null_value = fields_properties.at (column + 1).values.null;
              if (null_value.empty ())
                ss << "null";
              else
                ss << null_value;
            }
          else
            {
              auto data_type = data_types [column + 1];
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
                  write_type_as_ascii (ss, data_type, array_sizes [column+1],
                                       data.data () + offset, output_precision);
                }
            }
        }
      ss << " \n";
      os << ss.str ();
      ss.str ("");
    }
}
