#include "../../Table.hxx"

#include "../../Common.hxx"
#include "../../H5_to_Data_Type.hxx"
#include "../HDF5_Attribute.hxx"
#include "../HDF5_Property.hxx"

namespace {
const std::string read_description(const H5::DataSet &dataset,
                                   const std::string desc_label) {
    std::string desc_str;
    if (dataset.attrExists(desc_label.c_str())) {
        auto description = dataset.openAttribute(desc_label.c_str());
        if (description.getTypeClass() == H5T_STRING) {
            description.read(description.getDataType(), desc_str);
        }
    }
    return desc_str;
}

}  // namespace
////

namespace tablator {
Labeled_Properties read_metadata(const H5::DataSet &dataset);

std::vector<Column> read_column_metadata(const H5::H5Location &dataset,
                                         const std::string &section);


}  // namespace tablator

void tablator::Table::read_hdf5(const boost::filesystem::path &path) {
  // std::cout << "read_hdf5(), path, enter" << std::endl;
    H5::H5File file(path.string(), H5F_ACC_RDONLY);
    // FIXME: This needs to be generalized for multiple resources and
    // multiple tables
    H5::Group resource = file.openGroup("/RESOURCE_0");
  // std::cout << "read_hdf5(), before read_column_metadata() for PARAM" << std::endl;
    std::vector<tablator::Field> resource_params =
            read_column_metadata(resource, PARAM);

  // std::cout << "read_hdf5(), before dataset" << std::endl;
    H5::DataSet dataset = resource.openDataSet(resource.getObjnameByIdx(0).c_str());

  // std::cout << "read_hdf5(), before descriptions" << std::endl;
    set_description(read_description(dataset, DESCRIPTION));
    const auto resource_element_description =
            read_description(dataset, RESOURCE_ELEMENT_DESCRIPTION);
    const auto table_element_description =
            read_description(dataset, TABLE_ELEMENT_DESCRIPTION);

  // std::cout << "read_hdf5(), before table_element_params" << std::endl;
    std::vector<tablator::Field> table_element_params =
            read_column_metadata(dataset, PARAM);

    const auto metadata = read_metadata(dataset);

    // Distribute the mishmash of labeled_properties stored as metadata
    // between assorted class members at assorted levels.
    Labeled_Properties resource_element_labeled_properties;
    std::vector<Property> resource_element_trailing_infos;
    ATTRIBUTES resource_element_attributes;
    std::vector<Property> table_element_trailing_infos;
    ATTRIBUTES table_element_attributes;

  // std::cout << "read_hdf5(), before distribute_metadata()" << std::endl;
    distribute_metadata(resource_element_labeled_properties,
                        resource_element_trailing_infos, resource_element_attributes,
                        table_element_trailing_infos, table_element_attributes,
                        metadata);

  // std::cout << "read_hdf5(), before read_column_metadata() for FIELD" << std::endl;
    std::vector<tablator::Field> column_metadata =
            read_column_metadata(dataset, "FIELD");

    // FIXME: This does not handle field_properties
    // FIXME: This assumes that the first column is null_bitfield_flags
    auto compound = dataset.getCompType();
    if (column_metadata.size() != static_cast<size_t>(compound.getNmembers())) {
        throw std::runtime_error(
                "Inconsistent number of fields.  The 'FIELD' metadata lists " +
                std::to_string(column_metadata.size()) + " but the dataset has " +
                std::to_string(compound.getNmembers()));
    }

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};

    for (int i = 0; i < compound.getNmembers(); ++i) {
        H5::DataType datatype(compound.getMemberDataType(i));
        std::string name(compound.getMemberName(i));
        if (datatype.getClass() == H5T_STRING) {
		  // std::cout << "read_hdf5(), create column, H5T_String" << std::endl;
            tablator::append_column(columns, offsets, name, Data_Type::CHAR,
                                    datatype.getSize(),
                                    column_metadata[i].get_field_properties(), column_metadata[i].get_dynamic_array_flag());
        } else if (datatype.getClass() == H5T_ARRAY) {
            auto array_type = compound.getMemberArrayType(i);
            hsize_t ndims = array_type.getArrayNDims();
            if (ndims != 1) {
                throw std::runtime_error(
                        "Invalid number of dimensions when "
                        "reading a dataset.  Expected 1, "
                        "but got:" +
                        std::to_string(ndims));
            }
            array_type.getArrayDims(&ndims);
		  // std::cout << "read_hdf5(), create column, H5T_ARRAY" << std::endl;
            tablator::append_column(columns, offsets, name, H5_to_Data_Type(datatype),
                                    ndims, column_metadata[i].get_field_properties(), column_metadata[i].get_dynamic_array_flag());
        } else {
		  // std::cout << "read_hdf5(), create column, neither STRING nor ARRAY" << std::endl;
            tablator::append_column(columns, offsets, name, H5_to_Data_Type(datatype),
                                    1, column_metadata[i].get_field_properties(), false);
        }
    }

#if 0

    // We have to extract the data array and then modify it to include per-row array-size for dynamic (CHAR) arrays.
	// JTODO get hdf5_row_size;

	size_t num_rows = dataset.getSpace().getSimpleExtentNpoints();

	size_t hdf5_row_size = tablator::get_row_size_without_dynamic_array_sizes(offsets, columns);
	size_t hdf5_data_size = hdf5_row_size * num_rows;

	size_t tab_row_size = tablator::get_row_size(offsets);
	size_t tab_data_size = tab_row_size * num_rows;

    std::vector<uint8_t> hdf5_data_vec;
	// std::cout << "read_hdf5(), before resize(), hdf5_data_size: " << hdf5_data_size << std::endl;


    hdf5_data_vec.resize(hdf5_data_size);
	// std::cout << "read_hdf5(), before dataset.read()" << std::endl;
	uint8_t *hdf5_data_ptr = hdf5_data_vec.data();
    dataset.read(hdf5_data_ptr, compound);

	// std::cout << "read_hdf5(), after dataset.read()" << std::endl;

	std::vector<uint8_t> tab_data_vec;
	// std::cout << "read_hdf5(), before tab resize(), tab_data_size: " << tab_data_size << std::endl;

	tab_data_vec.resize(tab_data_size);
	uint8_t *tab_data_ptr = tab_data_vec.data();
	//	uint8_t *tab_data_ptr_orig = tab_data_ptr;

	// JTODO assume null_flags col is already populated
    const size_t null_flags_size((columns.size() + 6) / 8);

#if 0
    size_t hdf5_position = 0;
    size_t tab_position = 0;
#endif

	//    Row row(row_size(offsets));
    for (size_t r = 0; r < num_rows; ++r) {
	  //        row.fill_with_zeros();
#if 0
        size_t hdf5_row_offset = hdf5_position;
        size_t tab_row_offset = tab_position;


        hdf5_position += null_flags_size;
		position += null_flags_size;
#endif

	  // std::cout << "read_hdf5(), before null col" << std::endl;
		memcpy(tab_data_ptr, hdf5_data_ptr, null_flags_size);
	  // std::cout << "after_hdf5(), before null col" << std::endl;

		hdf5_data_ptr += null_flags_size;
		tab_data_ptr += null_flags_size;


		// JTODO do right thing by nulls
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
		  const auto &column = columns[col_idx];
		  bool dynamic_array_flag = column.get_dynamic_array_flag();
			size_t array_size = column.get_array_size();
			// std::cout << "read_hdf5(), array_size: " << array_size << std::endl;
			if (dynamic_array_flag) {
			  // std::cout << "read_hdf5(), col_idx: " << col_idx << ", dynamic: " << dynamic_array_flag << std::endl;
			  // JTODO hdf5 doesn't record per-row array_size, so add it.
			  *(reinterpret_cast<uint32_t *>(tab_data_ptr)) = array_size;
			  tab_data_ptr += sizeof(uint32_t);
			}

			const size_t data_type_size = data_size(column.get_type());
			size_t num_bytes_to_copy = array_size * data_type_size;
			// std::cout << "num_bytes_to_copy: " << num_bytes_to_copy << std::endl;
			memcpy(tab_data_ptr, hdf5_data_ptr, num_bytes_to_copy);

			hdf5_data_ptr += num_bytes_to_copy;
			tab_data_ptr += num_bytes_to_copy;
		} // end loop through columns
	} // end loop through rows
#else

    std::vector<uint8_t> data;
	// std::cout << " read_hdf5(), resize: " << tablator::get_row_size(offsets) *
	// dataset.getSpace().getSimpleExtentNpoints() << std::endl;

    data.resize(tablator::get_row_size(offsets) *
                dataset.getSpace().getSimpleExtentNpoints());
    dataset.read(data.data(), compound);

#endif


    std::vector<Table_Element> table_elements;
    const auto table_element =
	  Table_Element::Builder(columns, offsets, data)
                    .add_params(table_element_params)
                    .add_description(table_element_description)
                    .add_trailing_info_list(table_element_trailing_infos)
                    .add_attributes(table_element_attributes)
                    .build();
	// std::cout << "after table_element" << std::endl;

    add_resource_element(
            Resource_Element::Builder(table_element)
                    .add_params(resource_params)
                    .add_labeled_properties(resource_element_labeled_properties)
                    .add_description(resource_element_description)
                    .add_trailing_info_list(resource_element_trailing_infos)
                    .add_attributes(resource_element_attributes)
                    .build());
	// std::cout << "after resource_element" << std::endl;
}
