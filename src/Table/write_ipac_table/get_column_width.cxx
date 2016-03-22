#include "../../Table.hxx"

std::vector<size_t> tablator::Table::get_column_width () const
{
  std::vector<size_t> widths;
  for (auto &column: columns)
    {
      if (column.type == Data_Type::CHAR)
        {
          /// The minimum of 4 is to accomodate the length of the
          /// literals 'char' and 'null'.
          widths.push_back (std::max ((size_t)4,
                                      std::max (column.name.size (),
                                                column.array_size)));
        }
      else
        {
          /// buffer_size = 1 (sign) + 1 (leading digit) + 1 (decimal)
          /// + 1 (exponent sign) + 3 (exponent)
          const size_t buffer_size (7);
          widths.push_back (output_precision + buffer_size);
        }
    }
  return widths;
}
