#include "../../H5_to_Data_Type.hxx"
#include "../../Table.hxx"
#include "../HDF5_Property.hxx"
#include "../HDF5_Attribute.hxx"

namespace tablator
{
std::vector<std::pair<std::string, Property> >
read_metadata (const H5::DataSet &dataset);
}

void tablator::Table::read_hdf5 (const boost::filesystem::path &path)
{
  H5::H5File file (path.string (), H5F_ACC_RDONLY);
  H5::Group group = file.openGroup ("/");
  H5::DataSet dataset = file.openDataSet (group.getObjnameByIdx (0).c_str ());

  if (dataset.attrExists ("DESCRIPTION"))
    {
      auto description = dataset.openAttribute ("DESCRIPTION");
      if (description.getTypeClass () == H5T_STRING)
        {
          comments.resize (1);
          description.read (description.getDataType (), comments[0]);
        }
    }

  properties=read_metadata (dataset);
  
  // FIXME: This does not handle fields_properties
  // FIXME: This assumes that the first column is null_bitfield_flags
  auto compound = dataset.getCompType ();
  for (int i = 0; i < compound.getNmembers (); ++i)
    {
      H5::DataType datatype (compound.getMemberDataType (i));
      std::string name (compound.getMemberName (i));
      if (datatype.getClass () == H5T_STRING)
        {
          append_column (name, Data_Type::CHAR, datatype.getSize ());
        }
      else if (datatype.getClass () == H5T_ARRAY)
        {
          auto array_type = compound.getMemberArrayType (i);
          hsize_t ndims = array_type.getArrayNDims ();
          if (ndims != 1)
            { throw std::runtime_error ("Invalid number of dimensions when "
                                        "reading a dataset.  Expected 1, "
                                        "but got:" + std::to_string (ndims)); }
          array_type.getArrayDims (&ndims);
          append_column (name, H5_to_Data_Type (datatype), ndims);
        }
      else
        {
          append_column (name, H5_to_Data_Type (datatype));
        }
    }
  data.resize (row_size () * dataset.getSpace ().getSimpleExtentNpoints ());
  dataset.read (data.data (), compound);
}
