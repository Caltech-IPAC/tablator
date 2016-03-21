#include "../Table.hxx"

void tablator::Table::append_column_internal (const std::string &name,
                                              const H5::DataType &type,
                                              const size_t &array_size)
{
  size_t new_row_size=row_size + type.getSize ();
  auto new_data_types (data_types);
  new_data_types.emplace_back (H5_to_Data_Type (type));

  auto new_array_sizes (array_sizes);
  new_array_sizes.push_back(array_size);

  auto new_offsets (offsets);
  new_offsets.push_back (new_row_size);

  auto new_fields_properties (fields_properties);
  new_fields_properties.push_back (Field_Properties ());

  auto new_compound_type (compound_type);
  new_compound_type.setSize (new_row_size);
  new_compound_type.insertMember (name, row_size, type);

  /// Copy and swap for exception safety.
  row_size=new_row_size;
  using namespace std;
  swap (data_types, new_data_types);
  swap (array_sizes, new_array_sizes);
  swap (offsets, new_offsets);
  swap (fields_properties, new_fields_properties);
  swap (compound_type, new_compound_type);
}
