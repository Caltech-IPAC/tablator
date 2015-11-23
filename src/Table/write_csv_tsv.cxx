#include <iomanip>

#include "../Table.hxx"

void tablator::Table::write_csv_tsv (std::ostream &os, const char &separator)
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
        if (types[i]==H5::PredType::STD_I8LE)
          {
            /// Booleans are converted to integers
            os << static_cast<int>(data[offset]);
          }
        else if (types[i]==H5::PredType::STD_I16LE)
          {
            os << (*reinterpret_cast<const int16_t *>(data.data () + offset));
          }
        else if (types[i]==H5::PredType::STD_I32LE)
          {
            os << (*reinterpret_cast<const int32_t *>(data.data () + offset));
          }
        else if (types[i]==H5::PredType::STD_I64LE)
          {
            os << (*reinterpret_cast<const int64_t *>(data.data () + offset));
          }
        else if (types[i]==H5::PredType::IEEE_F32LE)
          {
            os << std::setprecision (output_precision)
               << (*reinterpret_cast<const float *>(data.data () + offset));
          }
        else if (types[i]==H5::PredType::IEEE_F64LE)
          {
            os << std::setprecision (output_precision)
               << (*reinterpret_cast<const double *>(data.data () + offset));
          }
        else if (types[i]==H5::PredType::C_S1)
          {
            /// The characters in the type can be shorter than the
            /// number of allowed bytes.  So add a .c_str() that will
            /// terminate the string at the first null.
            os << std::string (data.data () + offset,
                               compound_type.getMemberDataType (i).getSize ()).c_str();
          }
        os << (i == num_members - 1 ? '\n' : separator);
      }
}
