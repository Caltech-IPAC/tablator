#include <array>
#include <sstream>

#include "../../Table.hxx"

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

  H5::CompType votable (size_t(1));
  std::stringstream ss;
  std::vector<H5::StrType> strings;
  std::vector<H5::CompType> compound_types;
  table.write (data.data (), compound_type);
}
