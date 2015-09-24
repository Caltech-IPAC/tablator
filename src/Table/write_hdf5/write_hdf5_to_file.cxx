#include <vector>
#include <array>
#include <sstream>

#include "../../Table.hxx"

class HDF5_Property
{
public:
  const char *name;
  const char *value;
  hvl_t attributes;

  HDF5_Property (const char *Name, const char *Value, const hvl_t &Attributes)
    : name (Name), value (Value), attributes (Attributes) {}
};

class HDF5_Attribute
{
public:
  const char *name;
  const char *value;

  HDF5_Attribute (const char *Name, const char *Value)
    : name (Name), value (Value) {}
};


void tablator::Table::write_hdf5_to_file (H5::H5File &outfile) const
{
  std::array<hsize_t, 1> dims = { { size () } };
  H5::DataSpace dataspace (dims.size (), dims.data ());

  H5::DataSet table{ outfile.createDataSet ("table", compound_type,
                                            dataspace) };
  if (!comments.empty ())
    {
      std::string description;
      for (auto &c: comments)
        description+=c + '\n';
      if (!description.empty ())
        {
          description.resize (description.size () - 1);
          H5::StrType str_type (0, description.size ());
          H5::DataSpace attribute_space (H5S_SCALAR);
          H5::Attribute attribute
            = table.createAttribute ("DESCRIPTION", str_type, attribute_space);
          attribute.write (str_type, description.c_str ());
        }
    }

  H5::StrType hdf5_string (0,H5T_VARIABLE);
  H5::CompType hdf5_attribute_type (2*hdf5_string.getSize ());
  hdf5_attribute_type.insertMember ("name",0,hdf5_string);
  hdf5_attribute_type.insertMember ("value",hdf5_string.getSize (),hdf5_string);
  H5::VarLenType hdf5_attributes_type (&hdf5_attribute_type);

  H5::CompType hdf5_property_type (sizeof (HDF5_Property));
  hdf5_property_type.insertMember ("name",HOFFSET (HDF5_Property,name),
                                   hdf5_string);
  hdf5_property_type.insertMember ("value",HOFFSET (HDF5_Property,value),
                                   hdf5_string);
  hdf5_property_type.insertMember ("attributes",
                                   HOFFSET (HDF5_Property,attributes),
                                   hdf5_attributes_type);

  std::vector<std::vector<const char *> > strings;
  H5::VarLenType hdf5_properties_type (&hdf5_property_type);

  std::vector<HDF5_Property> hdf5_properties;

  for (auto &property : properties)
    {
      auto &p=property.second;

      if (!p.value.empty () || !p.attributes.empty ())
        {
          strings.emplace_back();
          std::vector<const char *> &s=*strings.rbegin ();
          for (auto &a: p.attributes)
            {
              std::cout << "\t" << a.first << " " << a.second << "\n";
              s.push_back(a.first.c_str ());
              s.push_back(a.second.c_str ());
            }
      
          hvl_t atts;
          atts.len=s.size ()/2;
          atts.p=s.data ();
          hdf5_properties.emplace_back (property.first.c_str (),
                                        p.value.c_str (),atts);
        }
    }

  hvl_t hdf5_props;
  hdf5_props.len=hdf5_properties.size ();
  hdf5_props.p=hdf5_properties.data ();

  H5::DataSpace property_space (H5S_SCALAR);
  H5::Attribute table_attribute
    = table.createAttribute ("METADATA", hdf5_properties_type, property_space);
  table_attribute.write (hdf5_properties_type, &hdf5_props);


  /// Write the data
  table.write (data.data (), compound_type);
}
