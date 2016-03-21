#include <iomanip>

#include "../Table.hxx"
#include "write_type_as_ascii.hxx"

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
        write_type_as_ascii (os, data_types[i], array_sizes[i],
                             data.data () + offset, output_precision);
        os << (i == num_members - 1 ? '\n' : separator);
      }
}
