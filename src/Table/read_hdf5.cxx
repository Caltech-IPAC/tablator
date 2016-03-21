#include <limits>

#include "../Table.hxx"
#include "HDF5_Property.hxx"
#include "HDF5_Attribute.hxx"

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

  if (dataset.attrExists ("METADATA"))
    {
      auto metadata = dataset.openAttribute ("METADATA");

      if (metadata.getTypeClass () == H5T_VLEN)
        {
          H5::VarLenType hdf5_properties_type = metadata.getVarLenType ();
          H5::DataType hdf5_properties_super
              = hdf5_properties_type.getSuper ();
          if (hdf5_properties_super.getClass () == H5T_COMPOUND)
            {
              H5::CompType hdf5_property_type (hdf5_properties_super.getId ());
              if (hdf5_property_type.getNmembers () == 3
                  && hdf5_property_type.getMemberClass (0) == H5T_STRING
                  && hdf5_property_type.getMemberClass (1) == H5T_STRING
                  && hdf5_property_type.getMemberClass (2) == H5T_VLEN)
                {
                  H5::VarLenType attributes_type
                      = hdf5_property_type.getMemberVarLenType (2);
                  H5::DataType attributes_super = attributes_type.getSuper ();
                  if (attributes_super.getClass () == H5T_COMPOUND)
                    {
                      H5::CompType attribute_type (attributes_super.getId ());
                      if (attribute_type.getNmembers () == 2
                          && attribute_type.getMemberClass (0) == H5T_STRING
                          && attribute_type.getMemberClass (1) == H5T_STRING)
                        {
                          hvl_t hdf5_props;
                          metadata.read (hdf5_properties_type, &hdf5_props);
                          for (size_t n = 0; n < hdf5_props.len; ++n)
                            {
                              HDF5_Property &hdf5_property
                                  = reinterpret_cast<HDF5_Property *>(
                                      hdf5_props.p)[n];
                              Property p (hdf5_property.value);
                              for (size_t i = 0;
                                   i < hdf5_property.attributes.len; ++i)
                                {
                                  HDF5_Attribute &hdf_attribute
                                      = reinterpret_cast<HDF5_Attribute *>(
                                          hdf5_property.attributes.p)[i];
                                  p.attributes.insert (std::make_pair (
                                      std::string (hdf_attribute.name),
                                      std::string (hdf_attribute.value)));
                                }
                              properties.push_back (std::make_pair (
                                  std::string (hdf5_property.name), p));
                            }
                        }
                    }
                }
            }
        }
    }

  // FIXME: This does not handle fields_properties
  // FIXME: This assumes that the first column is null_bitfield_flags
  auto compound = dataset.getCompType ();
  for (int i = 0; i < compound.getNmembers (); ++i)
    {
      H5::DataType datatype (compound.getMemberDataType (i));
      std::string name (compound.getMemberName (i));
      if (datatype.getClass () == H5T_STRING)
        {
          append_string_column (name, datatype.getSize ());
        }
      else if (datatype.getClass () == H5T_ARRAY)
        {
          append_array_column (name, compound.getMemberArrayType (i));
        }
      else
        {
          append_column_internal (name, datatype, 1);
        }
    }
  data.resize (row_size * dataset.getSpace ().getSimpleExtentNpoints ());
  dataset.read (data.data (), compound_type);
}
