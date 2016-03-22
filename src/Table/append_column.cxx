#include "../Table.hxx"

void tablator::Table::append_column (const Column &column)
{
  auto new_columns (columns);
  new_columns.push_back (column);
  size_t new_row_size = row_size () + new_columns.rbegin ()->data_size ();
  
  auto new_offsets (offsets);
  new_offsets.push_back (new_row_size);

  /// Copy and swap for exception safety.
  using namespace std;
  swap (columns, new_columns);
  swap (offsets, new_offsets);
}
