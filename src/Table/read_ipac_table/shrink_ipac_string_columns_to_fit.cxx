#include "../../Table.hxx"

#include <utility>
#include <stdexcept>

void tablator::Table::shrink_ipac_string_columns_to_fit (
    const std::vector<size_t> &column_widths)
{
  std::vector<size_t> new_offsets = { 0 };
  std::vector<Data_Type> new_data_types;
  std::vector<H5::StrType> new_string_types;
  H5::CompType new_compound_type (size_t (1));

  size_t new_row_size (0);
  for (size_t column = 0; column < offsets.size () - 1; ++column)
    {
      H5::DataType H5_type = compound_type.getMemberDataType (column);
      Data_Type data_type (H5_to_Data_Type (H5_type));
      size_t old_size = new_row_size;
      switch (data_type)
        {
          /// Strings are shrunk.  Everything else stays the same.
        case Data_Type::STRING:
          new_string_types.emplace_back (0, column_widths[column]);
          new_row_size += new_string_types.rbegin ()->getSize ();
          new_compound_type.setSize (new_row_size);
          new_compound_type.insertMember (compound_type.getMemberName (column),
                                          old_size,
                                          *new_string_types.rbegin ());
          break;
        default:
          new_row_size += H5_type.getSize ();
          new_compound_type.setSize (new_row_size);
          new_compound_type.insertMember (compound_type.getMemberName (column),
                                          old_size, H5_type);
          break;
        }
      new_offsets.push_back (new_row_size);
      new_data_types.push_back (data_type);
    }
  const size_t rows = num_rows ();
  //FIXME: Do this in place.
  std::vector<char> new_data (rows * new_row_size);
  size_t row_offset (0), new_row_offset (0);
  for (size_t row = 0; row < rows; ++row)
    {
      for (size_t column = 0; column < offsets.size () - 1; ++column)
        {
          std::copy (data.begin () + row_offset + offsets[column],
                     data.begin () + row_offset + offsets[column]
                     + (new_offsets[column + 1] - new_offsets[column]),
                     new_data.begin () + new_row_offset + new_offsets[column]);
        }
      row_offset += row_size;
      new_row_offset += new_row_size;
    }
  using namespace std;
  swap (data, new_data);
  swap (data_types, new_data_types);
  swap (offsets, new_offsets);
  swap (string_types, new_string_types);
  swap (compound_type, new_compound_type);
  swap (row_size, new_row_size);
}