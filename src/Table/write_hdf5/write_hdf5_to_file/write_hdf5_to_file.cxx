#include <array>

#include "../../../Table.hxx"
#include "../../../Data_Type_to_H5.hxx"

namespace tablator
{
void write_hdf5_columns (const std::vector<Column> &columns,
                         const std::string &column_type, H5::DataSet &table);
}

void tablator::Table::write_hdf5_to_file (H5::H5File &outfile) const
{
  std::array<hsize_t, 1> dims = { { num_rows () } };
  H5::DataSpace dataspace (dims.size (), dims.data ());

  std::vector<H5::StrType> string_types;
  std::vector<H5::ArrayType> array_types;
  H5::CompType compound_type (row_size ());
  std::set<std::string> unique_names;
  for (auto &column: columns)
    unique_names.insert(column.name);
  if (unique_names.size() != columns.size())
    throw std::runtime_error ("Duplicate column names are not allowed in HDF5 tables");
  for (size_t i=0; i<columns.size (); ++i)
    {
      if (columns[i].type == Data_Type::CHAR)
        {
          string_types.emplace_back (0, columns[i].array_size);
          compound_type.insertMember (columns[i].name, offsets[i],
                                      *string_types.rbegin ());
        }
      else if (columns[i].array_size != 1)
        {
          const hsize_t hsize (columns[i].array_size);
          array_types.emplace_back (Data_Type_to_H5 (columns[i].type), 1,
                                    &hsize);
          compound_type.insertMember (columns[i].name, offsets[i],
                                      *array_types.rbegin ());
        }
      else
        {
          compound_type.insertMember (columns[i].name, offsets[i],
                                      Data_Type_to_H5 (columns[i].type));
        }
    }
  H5::DataSet table{ outfile.createDataSet ("table", compound_type,
                                            dataspace) };
  if (!comments.empty ())
    {
      std::string description;
      for (auto &c : comments)
        description += c + '\n';
      if (!description.empty ())
        {
          description.resize (description.size () - 1);
          H5::StrType str_type (0, description.size ());
          H5::DataSpace attribute_space (H5S_SCALAR);
          H5::Attribute attribute = table.createAttribute (
              "DESCRIPTION", str_type, attribute_space);
          attribute.write (str_type, description.c_str ());
        }
    }
  write_hdf5_attributes (table);
  write_hdf5_columns (columns, "FIELD", table);
  write_hdf5_columns (params, "PARAM", table);
  table.write (data.data (), compound_type);
}
