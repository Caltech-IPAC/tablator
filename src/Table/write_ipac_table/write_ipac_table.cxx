#include <iomanip>
#include <inttypes.h>

#include "../../Table.hxx"

void TAP::Table::write_ipac_table (std::ostream &os) const
{
  const int num_members = compound_type.getNmembers ();
  write_ipac_table_header (os, num_members);

  const int record_width (20);
  os << "|";
  os << std::right;
  for (int i = 0; i < num_members; ++i)
    os << std::setw (record_width) << compound_type.getMemberName (i) << "|";
  os << "\n|";
  for (int i = 0; i < num_members; ++i)
    {
      os << std::setw (record_width);
      write_element_type (os, i);
      os << "|";
    }
  os << "\n|";
  for (auto &f : fields_properties)
    {
      os << std::setw (record_width);
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
    }
  os << "\n|";
  for (auto &f : fields_properties)
    {
      os << std::setw (record_width) << f.values.null;
      os << "|";
    }

  os << "\n";

  std::vector<char> buffer ((record_width + 1) * num_members + 3);

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
              /// Booleans are converted to integers
              current += snprintf (buffer.data () + current,
                                   buffer.size () - current, "%20d ",
                                   static_cast<int>(data[offset]));
              break;
            case Type::SHORT:
              current += snprintf (
                  buffer.data () + current, buffer.size () - current, "%20d ",
                  *reinterpret_cast<const int16_t *>(data.data () + offset));
              break;
            case Type::INT:
              current += snprintf (
                  buffer.data () + current, buffer.size () - current, "%20d ",
                  *reinterpret_cast<const int32_t *>(data.data () + offset));
              break;
            case Type::LONG:
              current += snprintf (
                  buffer.data () + current, buffer.size () - current,
                  // FIXME: This is not a portable way to print a 64
                  // bit int, but the standard way using PRId64 does
                  // not work.
                  "%20ld ",
                  *reinterpret_cast<const int64_t *>(data.data () + offset));
              break;
            case Type::FLOAT:
              // FIXME: Use Table::output_precision
              current += snprintf (
                  buffer.data () + current, buffer.size () - current,
                  "%20.13g ",
                  *reinterpret_cast<const float *>(data.data () + offset));
              break;
            case Type::DOUBLE:
              current += snprintf (
                  buffer.data () + current, buffer.size () - current,
                  "%20.13g ",
                  *reinterpret_cast<const double *>(data.data () + offset));
              break;
            case Type::STRING:
              const size_t string_size = offsets[i + 1] - offsets[i];
              for (size_t k = 0; k < record_width - string_size; ++k)
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
