#include <limits>

#include "../Table.hxx"
#include "HDF5_Property.hxx"
#include "HDF5_Attribute.hxx"

void tablator::Table::read_hdf5 (const boost::filesystem::path &path)
{
  H5::H5File file (path.string (), H5F_ACC_RDONLY);
  H5::Group group = file.openGroup ("/");
  H5::DataSet dataset = file.openDataSet (group.getObjnameByIdx (0).c_str ());
  compound_type = dataset.getCompType ();

  if (dataset.attrExists ("DESCRIPTION"))
    {
      auto description=dataset.openAttribute ("DESCRIPTION");
      if (description.getTypeClass ()==H5T_STRING)
        {
          comments.resize (1);
          description.read (description.getDataType (), comments[0]);
        }
    }

  if (dataset.attrExists ("METADATA"))
    {
      auto metadata=dataset.openAttribute ("METADATA");

      if (metadata.getTypeClass ()==H5T_VLEN)
        {
          H5::VarLenType hdf5_properties_type=metadata.getVarLenType ();
          H5::DataType hdf5_properties_super=hdf5_properties_type.getSuper ();
          if (hdf5_properties_super.getClass ()==H5T_COMPOUND)
            {
              H5::CompType hdf5_property_type(hdf5_properties_super.getId ());
              if (hdf5_property_type.getNmembers ()==3
                  && hdf5_property_type.getMemberClass(0)==H5T_STRING
                  && hdf5_property_type.getMemberClass(1)==H5T_STRING
                  && hdf5_property_type.getMemberClass(2)==H5T_VLEN)
                {
                  H5::VarLenType attributes_type=
                    hdf5_property_type.getMemberVarLenType (2);
                  H5::DataType attributes_super=attributes_type.getSuper ();
                  if (attributes_super.getClass ()==H5T_COMPOUND)
                    {
                      H5::CompType attribute_type(attributes_super.getId ());
                      if (attribute_type.getNmembers ()==2
                          && attribute_type.getMemberClass(0)==H5T_STRING
                          && attribute_type.getMemberClass(1)==H5T_STRING)
                        {
                          hvl_t hdf5_props;
                          metadata.read(hdf5_properties_type, &hdf5_props);
                          for (size_t n=0; n<hdf5_props.len; ++n)
                            {
                              HDF5_Property &hdf5_property=
                                reinterpret_cast<HDF5_Property*>(hdf5_props.p)[n];
                              Property p(hdf5_property.value);
                              for (size_t i=0; i<hdf5_property.attributes.len;
                                   ++i)
                                {
                                  HDF5_Attribute &hdf_attribute=
                                    reinterpret_cast<HDF5_Attribute*>
                                    (hdf5_property.attributes.p)[i];
                                  p.attributes.insert
                                    (std::make_pair
                                     (std::string(hdf_attribute.name),
                                      std::string(hdf_attribute.value)));
                                }
                              properties.push_back(std::make_pair
                                                   (std::string
                                                    (hdf5_property.name),p));
                            }
                        }
                    }
                }
            }
        }
    }
  
  offsets.clear ();
  types.clear ();
  size_t offset{ 0 };

  // FIXME: This assumes that the first column is null_bitfield_flags
  for (int i = 0; i < compound_type.getNmembers (); ++i)
    {
      /// There is no way to get a PredType from the compound type.
      /// So we have to manually check all of the possibilities and
      /// hope it is string if none match.
      H5::DataType d = compound_type.getMemberDataType (i);
      if (d == H5::PredType::STD_I8LE)
        types.push_back (H5::PredType::STD_I8LE);
      else if (d == H5::PredType::STD_U8LE)
        types.push_back (H5::PredType::STD_U8LE);
      else if (d == H5::PredType::STD_I16LE)
        types.push_back (H5::PredType::STD_I16LE);
      else if (d == H5::PredType::STD_U16LE)
        types.push_back (H5::PredType::STD_U16LE);
      else if (d == H5::PredType::STD_I32LE)
        types.push_back (H5::PredType::STD_I32LE);
      else if (d == H5::PredType::STD_U32LE)
        types.push_back (H5::PredType::STD_U32LE);
      else if (d == H5::PredType::STD_I64LE)
        types.push_back (H5::PredType::STD_I64LE);
      else if (d == H5::PredType::STD_U64LE)
        types.push_back (H5::PredType::STD_U64LE);
      else if (d == H5::PredType::IEEE_F32LE)
        types.push_back (H5::PredType::IEEE_F32LE);
      else if (d == H5::PredType::IEEE_F64LE)
        types.push_back (H5::PredType::IEEE_F64LE);
      else
        {
          // Do we have to create a string_type since compound_type
          // lives on???
          string_types.push_back (compound_type.getMemberStrType (i));
          types.push_back (H5::PredType::C_S1);
        }
      offsets.push_back (offset);
      offset += d.getSize ();
      fields_properties.push_back (Field_Properties (
          std::string (""), {}));
    }
  offsets.push_back (offset);
  row_size = offset;
  data.resize (row_size * dataset.getSpace ().getSimpleExtentNpoints ());
  dataset.read (data.data (), compound_type);
}
