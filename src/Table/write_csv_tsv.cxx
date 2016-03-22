#include <iomanip>

#include "../Table.hxx"
#include "write_type_as_ascii.hxx"

void tablator::Table::write_csv_tsv (std::ostream &os, const char &separator)
    const
{
  const int num_members = columns.size ();
  if (num_members == 0)
    return;
  /// Skip null_bitfield_flags
  for (int i = 1; i < num_members; ++i)
    os << columns[i].name
       << (i == num_members - 1 ? '\n' : separator);

  for (size_t j = 0; j < data.size (); j += row_size ())
    for (int i = 1; i < num_members; ++i)
      {
        size_t offset = offsets[i] + j;
        write_type_as_ascii (os, columns[i].type, columns[i].array_size,
                             data.data () + offset, output_precision);
        os << (i == num_members - 1 ? '\n' : separator);
      }
}
