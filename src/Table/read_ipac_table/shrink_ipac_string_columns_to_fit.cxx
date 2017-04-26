#include "../../Table.hxx"

#include <utility>
#include <stdexcept>

void tablator::Table::shrink_ipac_string_columns_to_fit (
    const std::vector<size_t> &column_widths)
{
  std::vector<size_t> new_offsets = { 0 };
  std::vector<Column> new_columns (columns);

  size_t new_row_size (0);
  for (size_t i = 0; i < columns.size (); ++i)
    {
      if (columns[i].type == Data_Type::CHAR)
        {
          new_columns[i].array_size = column_widths[i];
        }
      new_row_size += new_columns[i].data_size ();
      new_offsets.push_back (new_row_size);
    }
  const size_t rows = num_rows ();
  // FIXME: Do this in place.
  std::vector<uint8_t> new_data (rows * new_row_size);
  size_t row_offset (0), new_row_offset (0);
  for (size_t row = 0; row < rows; ++row)
    {
      for (size_t column = 0; column < offsets.size () - 1; ++column)
        {
          std::copy (data.begin () + row_offset + offsets[column],
                     data.begin () + row_offset + offsets[column]
                     + new_columns[column].data_size (),
                     new_data.begin () + new_row_offset + new_offsets[column]);
        }
      row_offset += row_size ();
      new_row_offset += new_row_size;
    }
  using namespace std;
  swap (data, new_data);
  swap (columns, new_columns);
  swap (offsets, new_offsets);
}
