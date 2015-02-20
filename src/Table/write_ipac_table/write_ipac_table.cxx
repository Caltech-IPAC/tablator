#include <iomanip>
#include <inttypes.h>
#include <limits>
#include <cmath>

#include "../../Table.hxx"

void Tablator::Table::write_ipac_table (std::ostream &os) const
{
  const int num_members = compound_type.getNmembers ();
  write_ipac_table_header (os, num_members);
  int i = 0, total_record_width = 0;

  os << "|";
  os << std::right;
  for (int i = 0; i < num_members; ++i)
    {
      total_record_width += (ipac_column_widths[i] + 1);
      os << std::setw (ipac_column_widths[i])
         << compound_type.getMemberName (i) << "|";
    }
  os << "\n|";

  for (int i = 0; i < num_members; ++i)
    {
      os << std::setw (ipac_column_widths[i]);
      write_element_type (os, i);
      os << "|";
    }

  if (fields_properties.size () > 0)
    os << "\n|";
  else
    os << "\n";

  i = 0;
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
  for (size_t j = 0; j < data.size (); j += compound_type.getSize ())
    {
      int current = 0;
      current += snprintf (buffer.data () + current, buffer.size () - current,
                           " ");
      for (int i = 0; i < num_members; ++i)
        {
          size_t offset = offsets[i] + j;
          switch (types[i])
            {
            case Type::BOOLEAN:
              /// FIXME: assign a reasonable null value late
              /// Booleans are converted to integers
              if (static_cast<int>(data[offset]) == -9)
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      "%*s ", ipac_column_widths[i], nulls[i].c_str ());
                }
              else
                {
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*d ",
                                       ipac_column_widths[i],
                                       static_cast<int>(data[offset]));
                }
              break;

            case Type::SHORT:
              if (*reinterpret_cast<const int16_t *>(data.data () + offset)
                  == std::numeric_limits<int16_t>::max ())
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      "%*s ", ipac_column_widths[i], nulls[i].c_str ());
                }
              else
                {
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*d ",
                                       ipac_column_widths[i],
                                       *reinterpret_cast<const int16_t *>(
                                           data.data () + offset));
                }
              break;

            case Type::INT:
              if (*reinterpret_cast<const int32_t *>(data.data () + offset)
                  == std::numeric_limits<int32_t>::max ())
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      "%*s ", ipac_column_widths[i], nulls[i].c_str ());
                }
              else
                {
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*d ",
                                       ipac_column_widths[i],
                                       *reinterpret_cast<const int32_t *>(
                                           data.data () + offset));
                }
              break;

            case Type::LONG:
              if (*reinterpret_cast<const int64_t *>(data.data () + offset)
                  == std::numeric_limits<int64_t>::max ())
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      "%*s ", ipac_column_widths[i], nulls[i].c_str ());
                }
              else
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      // FIXME: This is not a portable way to print a 64
                      // bit int, but the standard way using PRId64 does
                      // not work.
                      "%*ld ", ipac_column_widths[i],
                      *reinterpret_cast<const int64_t *>(data.data ()
                                                         + offset));
                }
              break;

            case Type::FLOAT:
              // FIXME: Use Table::output_precision
              if (*reinterpret_cast<const float *>(data.data () + offset)
                  == std::numeric_limits<float>::max ())
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      "%*s ", ipac_column_widths[i], nulls[i].c_str ());
                }
              else
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      "%*.13g ", ipac_column_widths[i],
                      *reinterpret_cast<const float *>(data.data () + offset));
                }
              break;

            case Type::DOUBLE:
              if (*reinterpret_cast<const double *>(data.data () + offset)
                  == std::numeric_limits<double>::has_quiet_NaN)
                {
                  current += snprintf (
                      buffer.data () + current, buffer.size () - current,
                      "%*s ", ipac_column_widths[i], nulls[i].c_str ());
                }
              else
                {
                  current += snprintf (buffer.data () + current,
                                       buffer.size () - current, "%*.13g ",
                                       ipac_column_widths[i],
                                       *reinterpret_cast<const double *>(
                                           data.data () + offset));
                }
              break;

            case Type::STRING:
              const size_t string_size = offsets[i + 1] - offsets[i];
              for (size_t k = 0; k < ipac_column_widths[i] - string_size; ++k)
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
      current += snprintf (buffer.data () + current, buffer.size () - current,
                           "\n");
      os.write (buffer.data (), current);
    }
}
