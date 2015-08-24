#include <limits>

#include "../Table.hxx"

void tablator::Table::read_hdf5 (const boost::filesystem::path &path)
{
  H5::H5File file (path.string (), H5F_ACC_RDONLY);
  H5::Group group = file.openGroup ("/");
  H5::DataSet dataset = file.openDataSet (group.getObjnameByIdx (0).c_str ());
  compound_type = dataset.getCompType ();

  // TODO: handle properties and comments
  
  offsets.clear ();
  types.clear ();
  size_t offset{ 0 };

  for (int i = 0; i < compound_type.getNmembers (); ++i)
    {
      H5::DataType d = compound_type.getMemberDataType (i);
      if (d == H5::PredType::NATIVE_UCHAR)
        types.push_back (Type::BOOLEAN);
      else if (d == H5::PredType::NATIVE_INT16)
        types.push_back (Type::SHORT);
      else if (d == H5::PredType::NATIVE_INT32)
        types.push_back (Type::INT);
      else if (d == H5::PredType::NATIVE_INT64)
        types.push_back (Type::LONG);
      else if (d == H5::PredType::NATIVE_FLOAT)
        types.push_back (Type::FLOAT);
      else if (d == H5::PredType::NATIVE_DOUBLE)
        types.push_back (Type::DOUBLE);
      else
        {
          // Do we have to create a string_type since compound_type
          // lives on???
          string_types.push_back (compound_type.getMemberStrType (i));
          types.push_back (Type::STRING);
        }
      offsets.push_back (offset);
      offset += d.getSize ();
      fields_properties.push_back (Field_Properties (
          std::string (""), { { "unit", " " } }));
    }
  offsets.push_back (offset);
  row_size = offset;
  data.resize (row_size * dataset.getSpace ().getSimpleExtentNpoints ());
  dataset.read (data.data (), compound_type);
}
