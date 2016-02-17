#include <utility>

#include "../../Table.hxx"

void tablator::Table::shrink_ipac_string_columns_to_fit
(const std::vector<size_t> &array_sizes)
{
  std::vector<size_t> new_offsets={0};
  std::vector<H5::StrType> new_string_types;
  H5::CompType new_compound_type (size_t (1));

  size_t new_row_size (0);
  for (size_t column=0; column < offsets.size ()-1; ++column)
    {
      H5::DataType datatype=compound_type.getMemberDataType (column);
      size_t old_size=new_row_size;
      if (datatype.getClass ()!=H5T_STRING)
        {
          new_row_size+=datatype.getSize ();
          new_compound_type.setSize (new_row_size);
          new_compound_type.insertMember (compound_type.getMemberName (column),
                                          old_size, datatype);
        }
      else
        {
          new_string_types.emplace_back (0, array_sizes[column]);
          new_row_size+=new_string_types.rbegin ()->getSize ();
          new_compound_type.setSize (new_row_size);
          new_compound_type.insertMember (compound_type.getMemberName (column),
                                          old_size,
                                          *new_string_types.rbegin ());
        }
      new_offsets.push_back (new_row_size);
    }
  const size_t rows=num_rows ();
  std::vector<char> new_data (rows * new_row_size);
  size_t row_offset (0), new_row_offset (0);
  for (size_t row=0; row < rows; ++row)
    {
      for (size_t column=0; column < offsets.size ()-1; ++column)
        {
          std::copy (data.begin () + row_offset + offsets[column],
                     data.begin () + row_offset + offsets[column]
                     + (new_offsets[column+1] - new_offsets[column]),
                     new_data.begin () + new_row_offset + new_offsets[column]);
        }
      row_offset+=row_size;
      new_row_offset+=new_row_size;
    }
  using namespace std;
  swap (data, new_data);
  swap (new_offsets, offsets);
  swap (new_string_types, string_types);
  swap (new_compound_type, compound_type);
  swap (new_row_size, row_size);
}
