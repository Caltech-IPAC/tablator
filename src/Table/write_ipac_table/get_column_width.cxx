#include "../../Table.hxx"

std::vector<size_t> tablator::Table::get_column_width () const
{
  std::vector<size_t> widths;
  auto column(std::next(columns.begin()));
  /// First column is the null bitfield flags, which are not written
  /// out in ipac_tables.
  widths.push_back(0);
  for (; column!=columns.end(); ++column)
    {
      if (column->type == Data_Type::CHAR)
        {
          /// The minimum of 4 is to accomodate the length of the
          /// literals 'char' and 'null'.
          widths.push_back (std::max ((size_t)4,
                                      std::max (column->name.size (),
                                                column->array_size)));
        }
      else
        {
          /// buffer_size = 1 (sign) + 1 (leading digit) + 1 (decimal)
          /// + 1 (exponent sign) + 3 (exponent)
          const size_t buffer_size (7);
          size_t width
            (std::max (column->name.size ()
                       + (column->array_size==1 ? 0 :
                          1 + std::to_string(column->array_size-1).size()),
                       output_precision + buffer_size));
          for (size_t element=0; element<column->array_size; ++element)
            { widths.push_back(width); }
        }
    }
  return widths;
}
