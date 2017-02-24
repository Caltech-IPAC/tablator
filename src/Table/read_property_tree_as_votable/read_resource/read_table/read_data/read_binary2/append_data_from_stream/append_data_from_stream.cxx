#include "../is_null_MSb.hxx"
#include "../../../../VOTable_Field.hxx"
#include "../../../../../../../Table.hxx"

#include <boost/spirit/include/qi.hpp>

namespace tablator
{
void insert_swapped (const size_t &column_offset,
                     const Data_Type &data_type,
                     const size_t &array_size,
                     const std::vector<uint8_t> &stream,
                     const size_t &old_position, Row &row);
inline void insert_swapped (const size_t &column_offset,
                            const Column &column,
                            const std::vector<uint8_t> &stream,
                            const size_t &old_position, Row &row)
{
  return insert_swapped(column_offset, column.type,
                        column.array_size, stream, old_position, row);
}

void Table::append_data_from_stream
(const std::vector<uint8_t> &stream, const size_t &num_rows,
 const std::vector<VOTable_Field> &fields)
{
  const size_t null_flags_size ((columns.size() + 6)/8);
  size_t position (0);
  Row row (row_size());
  for (size_t r=0; r<num_rows; ++r)
    {
      row.set_zero ();
      size_t row_offset (position);
      position += null_flags_size;
      for (size_t column = 1; column < columns.size(); ++column)
        {
          if (is_null_MSb(stream,row_offset,column))
            {
              row.set_null (columns[column].type,
                            columns[column].array_size,
                            column, offsets[column],
                            offsets[column + 1]);
              if (fields[column].is_array_dynamic)
                position += sizeof(uint32_t);
              else
                position += data_size(columns[column].type)
                  * columns[column].array_size;
            }
          else
            {
              if (fields[column].is_array_dynamic)
                {
                  auto begin = stream.begin();
                  std::advance (begin, position);
                  auto end = stream.begin();
                  position += sizeof(uint32_t);
                  std::advance (end, position);
                  uint32_t dynamic_array_size (0);
                  boost::spirit::qi::parse
                    (begin, end, boost::spirit::qi::big_dword,
                     dynamic_array_size);
                  insert_swapped(offsets[column],
                                 columns[column].type,
                                 dynamic_array_size, stream,
                                 position, row);
                  position += data_size (columns[column].type)
                    * dynamic_array_size;
                }
              else
                {
                  insert_swapped(offsets[column], columns[column],
                                 stream, position, row);
                  position += columns[column].array_size
                    * data_size(columns[column].type);
                }
            }
        }
      if (position <= stream.size())
        append_row (row);
    }
}
}
