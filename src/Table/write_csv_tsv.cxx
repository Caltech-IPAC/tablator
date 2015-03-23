#include <iomanip>

#include "../Table.hxx"

void Tablator::Table::write_csv_tsv (std::ostream &os, const char &separator)
    const
{
  const int num_members = compound_type.getNmembers ();
  if (num_members == 0)
    return;
  /// Skip null_bitfield_flags
  for (int i = 1; i < num_members; ++i)
    os << compound_type.getMemberName (i)
       << (i == num_members - 1 ? '\n' : separator);

  for (size_t j = 0; j < data.size (); j += compound_type.getSize ())
    for (int i = 1; i < num_members; ++i)
      {
        size_t offset = offsets[i] + j;
        switch (types[i])
          {
          case Type::BOOLEAN:
            /// Booleans are converted to integers
            os << static_cast<int>(data[offset]);
            break;
          case Type::SHORT:
            os << (*reinterpret_cast<const int16_t *>(data.data () + offset));
            break;
          case Type::INT:
            os << (*reinterpret_cast<const int32_t *>(data.data () + offset));
            break;
          case Type::LONG:
            os << (*reinterpret_cast<const int64_t *>(data.data () + offset));
            break;
          case Type::FLOAT:
            os << std::setprecision (output_precision)
               << (*reinterpret_cast<const float *>(data.data () + offset));
            break;
          case Type::DOUBLE:
            os << std::setprecision (output_precision)
               << (*reinterpret_cast<const double *>(data.data () + offset));
            break;
          case Type::STRING:
            os << std::string (data.data () + offset,
                               compound_type.getMemberDataType (i).getSize ());
          }
        os << (i == num_members - 1 ? '\n' : separator);
      }
}
