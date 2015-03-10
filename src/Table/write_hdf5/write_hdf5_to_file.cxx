#include <array>

#include "../../Table.hxx"

void Tablator::Table::write_hdf5_to_file (H5::H5File &outfile) const
{
  std::array<hsize_t, 1> dims = { { size () } };
  H5::DataSpace dataspace (dims.size (), dims.data ());

  H5::DataSet table{ outfile.createDataSet ("table", compound_type,
                                            dataspace) };
  for (auto &p : flatten_properties ())
    {
      H5::StrType str_type (0, p.second.size ());
      H5::DataSpace attribute_space (H5S_SCALAR);
      H5::Attribute attribute
          = table.createAttribute (p.first, str_type, attribute_space);
      attribute.write (str_type, p.second.c_str ());
    }
  table.write (data.data (), compound_type);
}
