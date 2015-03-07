#include <iomanip>
#include <inttypes.h>
#include <limits>
#include <cmath>

#include "../../Table.hxx"

void Tablator::Table::write_ipac_table (std::ostream &os) const
{
  std::vector<size_t> ipac_column_widths=get_column_width ();

  const size_t num_members = types.size ()-1;
  write_ipac_table_header (os, num_members);
  int total_record_width = 0;

  os << "|";
  os << std::right;
  for (size_t i = 0; i < num_members; ++i)
    {
      total_record_width += (ipac_column_widths[i] + 1);
      os << std::setw (ipac_column_widths[i])
         << compound_type.getMemberName (i) << "|";
    }
  os << "\n|";

  for (size_t i = 0; i < num_members; ++i)
    {
      os << std::setw (ipac_column_widths[i]);
      write_element_type (os, i);
      os << "|";
    }

  if (fields_properties.size () > 0)
    os << "\n|";
  else
    os << "\n";

  size_t i = 0;
  for (auto &f : fields_properties)
    {
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
  if (fields_properties.size () > 0)
    os << "\n|";

  i = 0;
  for (auto &f : fields_properties)
    {
      os << std::setw (ipac_column_widths[i]);
      auto null = f.attributes.find ("null");
      if (null == f.attributes.end ())
        {
          os << " ";
        }
      else
        {
          os << null->second;
        }
      os << "|";
      ++i;
    }
  if (fields_properties.size () > 0)
    os << "\n";

  std::vector<char> buffer (total_record_width + 4);
  for (size_t row_offset = 0; row_offset < data.size (); row_offset += row_size)
    {
      int current = 0;
      current += snprintf (buffer.data () + current, buffer.size () - current,
                           " ");
      /// Skip the null bitfield flag
      for (size_t column = 1; column < num_members; ++column)
        {
          size_t offset = offsets[column] + row_offset;
          if (is_null (row_offset,column))
            {
              current += snprintf (
                                   buffer.data () + current, buffer.size () - current,
                                   "%*s ", ipac_column_widths[column], "null");
            }
          else
            {
              switch (types[column])
                {
                case Type::BOOLEAN:
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*d ",
                                       ipac_column_widths[column],
                                       static_cast<int>(data[offset]));
                  break;

                case Type::SHORT:
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*d ",
                                       ipac_column_widths[column],
                                       *reinterpret_cast<const int16_t *>(
                                                                          data.data () + offset));
                  break;

                case Type::INT:
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*d ",
                                       ipac_column_widths[column],
                                       *reinterpret_cast<const int32_t *>(
                                                                          data.data () + offset));
                  break;

                case Type::LONG:
                  current += snprintf (
                                       buffer.data () + current, buffer.size () - current,
                                       // FIXME: This is not a portable way to print a 64
                                       // bit int, but the standard way using PRId64 does
                                       // not work.
                                       "%*ld ", ipac_column_widths[column],
                                       *reinterpret_cast<const int64_t *>(data.data ()
                                                                          + offset));
                  break;

                case Type::FLOAT:
                  // FIXME: Use Table::output_precision
                  current += snprintf (
                                       buffer.data () + current, buffer.size () - current,
                                       "%*.13g ", ipac_column_widths[column],
                                       *reinterpret_cast<const float *>(data.data () + offset));
                  break;

                case Type::DOUBLE:
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*.13g ",
                                       ipac_column_widths[column],
                                       *reinterpret_cast<const double *>(
                                                                         data.data () + offset));
                  break;

                case Type::STRING:
                  const size_t string_size = offsets[column + 1] - offsets[column];
                  for (size_t k = 0; k < ipac_column_widths[column] - string_size; ++k)
                    {
                      buffer[current] = ' ';
                      ++current;
                    }
                  for (size_t k = offset; k < offset + string_size; ++k)
                    {

                      buffer[current] = data[k];
                      ++current;
                    }
                  buffer[current] = ' ';
                  ++current;
                  break;
                }
            }
        }
      current += snprintf (buffer.data () + current, buffer.size () - current,
                           "\n");
      os.write (buffer.data (), current);
    }
}
