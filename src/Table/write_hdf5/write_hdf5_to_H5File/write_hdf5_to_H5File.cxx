#include "../../../Table.hxx"

#include <array>

#include "../../../Common.hxx"
#include "../../../Data_Type_to_H5.hxx"

namespace {
void write_description(const H5::DataSet &h5_table, const std::string &label,
                       const std::string &desc_str) {
    if (!desc_str.empty()) {
        H5::StrType str_type(0, desc_str.size());
        H5::DataSpace attribute_space(H5S_SCALAR);
        H5::Attribute attribute =
                h5_table.createAttribute(label, str_type, attribute_space);
        attribute.write(str_type, desc_str.c_str());
    }
}
}  // namespace

namespace tablator {
void write_hdf5_columns(const std::vector<Column> &columns,
                        const std::string &column_type, H5::H5Location &table);
}

void tablator::Table::write_hdf5_to_H5File(H5::H5File &outfile) const {
    /// We map IVOA RESOURCE's to H5::Group's.  Group's must have names
    /// and they must be unique, so we can not use RESOURCE's name
    /// attribute.
    // FIXME: This needs to be generalized for multiple resources
    H5::Group group(outfile.createGroup("RESOURCE_0"));
	// std::cout << "write_hdf5_to_H5File(), before write_hdf5_columns(), RESOURCE PARAM" << std::endl;

    write_hdf5_columns(get_resource_element_params(), PARAM, group);
	// std::cout << "write_hdf5_to_H5File(), after write_hdf5_columns(), RESOURCE PARAM" << std::endl;

    std::array<hsize_t, 1> dims = {{get_num_rows()}};
    H5::DataSpace dataspace(dims.size(), dims.data());

    const auto &columns = get_columns();
    const auto &offsets = get_offsets();

	// std::cout << "write_hdf5_to_H5File(), before string_types()" << std::endl;
    std::vector<H5::StrType> string_types;
    std::vector<H5::ArrayType> array_types;
    H5::CompType compound_type(get_row_size());
    std::vector<std::string> unique_names;

    unique_names.reserve(columns.size());
    for (auto &column : columns) {
        unique_names.push_back(column.get_name());
    }
	// std::cout << "write_hdf5_to_H5File(), before unique names" << std::endl;

    std::sort(unique_names.begin(), unique_names.end());
    if (std::unique(unique_names.begin(), unique_names.end()) != unique_names.end()) {
        throw std::runtime_error(
                "Duplicate column names are not "
                "allowed in HDF5 tables");
    }
	// std::cout << "write_hdf5_to_H5File(), before loop through columns" << std::endl;
    for (size_t i = 0; i < columns.size(); ++i) {
	  // std::cout << "top of column loop, i: " << i << std::endl;
        if (columns[i].get_type() == Data_Type::CHAR) {
		  // std::cout << "type char" << std::endl;
            string_types.emplace_back(0, columns[i].get_array_size());
            compound_type.insertMember(columns[i].get_name(), offsets[i],
                                       *string_types.rbegin());
        } else if (columns[i].get_array_size() != 1) {
		  // std::cout << "array_size > 1" << std::endl;  // JTODO flag
            const hsize_t hsize(columns[i].get_array_size());
            array_types.emplace_back(Data_Type_to_H5(columns[i].get_type()), 1, &hsize);
            compound_type.insertMember(columns[i].get_name(), offsets[i],
                                       *array_types.rbegin());
        } else {
		  // std::cout << "array_size 1, not char" << std::endl;  // JTODO flag
            compound_type.insertMember(columns[i].get_name(), offsets[i],
                                       Data_Type_to_H5(columns[i].get_type()));
        }
    }
    // FIXME: This needs to be generalized for multiple tables
    H5::DataSet h5_table{group.createDataSet("TABLE_0", compound_type, dataspace)};
	// std::cout << "write_hdf5_to_H5File(), before comments" << std::endl;
    const auto &comments = get_comments();
    const auto &description = get_description();
    if (!description.empty() || !comments.empty()) {
        std::string description_and_comments;
        if (!description.empty()) {
            description_and_comments.assign(description + "\n");
        }
        for (auto &c : comments) {
            description_and_comments += c + '\n';
        }
        description_and_comments.resize(description_and_comments.size() - 1);
        write_description(h5_table, DESCRIPTION, description_and_comments);
    }
	// std::cout << "write_hdf5_to_H5File(), before description" << std::endl;

    write_description(h5_table, RESOURCE_ELEMENT_DESCRIPTION,
                      get_results_resource_element().get_description());
    write_description(h5_table, TABLE_ELEMENT_DESCRIPTION,
                      get_main_table_element().get_description());

    write_hdf5_attributes(h5_table);

	// std::cout << "write_hdf5_to_H5File(), before write_hdf5_columns(), TABLE_0 FIELD" << std::endl;
    write_hdf5_columns(columns, FIELD, h5_table);
	// std::cout << "write_hdf5_to_H5File(), after write_hdf5_columns(), TABLE_0 FIELD" << std::endl;
    // JTODO Top-level params? (Master doesn't.)

	// std::cout << "get_table_element_params().size(): " << get_table_element_params().size() << std::endl;

	// std::cout << "write_hdf5_to_H5File(), before write_hdf5_columns(), TABLE_0 PARAM" << std::endl;
	write_hdf5_columns(get_table_element_params(), PARAM, h5_table);
	// std::cout << "write_hdf5_to_H5File(), after write_hdf5_columns(), TABLE_0 PARAM" << std::endl;

#if 0
	// JTODO Create version of get_data().data() that doesn't include size of dynamic arrays.
    std::vector<uint8_t> hdf5_data_vec;

	size_t num_rows = get_num_rows();
	size_t hdf5_row_size = tablator::get_row_size_without_dynamic_array_sizes(offsets, columns);
	size_t hdf5_data_size = hdf5_row_size * num_rows;

	// std::cout << "write_hdf5_to_H5File(), before resize(), hdf5_data_size: " << hdf5_data_size << std::endl;

    hdf5_data_vec.resize(hdf5_data_size);
	std::fill(hdf5_data_vec.begin(), hdf5_data_vec.end(), 0);
	uint8_t *hdf5_data_ptr = hdf5_data_vec.data();

	// std::cout << "tab_data size: " << get_data().size() << std::endl;
	const uint8_t *tab_data_ptr = get_data().data();

	// std::cout << "write_hdf5_to_H5File(), before writing to hdf5_data_ptr" << std::endl;

	// JTODO do right thing by nulls

    const size_t null_flags_size((columns.size() + 6) / 8);
    for (size_t r = 0; r < num_rows; ++r) {

	  // std::cout << "write_hdf5_to_H5File(), before null col" << std::endl;
		memcpy(hdf5_data_ptr, tab_data_ptr, null_flags_size);
	  // std::cout << "write_hdf5_to_H5File(), after null col" << std::endl;
		hdf5_data_ptr += null_flags_size;
		tab_data_ptr += null_flags_size;

        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
		  const auto &column = columns[col_idx];
		  bool dynamic_array_flag = column.get_dynamic_array_flag();
			size_t array_size = column.get_array_size();
			// std::cout << "write_hdf5(), array_size: " << array_size << std::endl;
			if (dynamic_array_flag) {
			  // std::cout << "write_hdf5(), col_idx: " << col_idx << ", dynamic: " << dynamic_array_flag << std::endl;
			  // JTODO hdf5 doesn't record per-row array_size, so skip it.
			  tab_data_ptr += sizeof(uint32_t);
			}

			const size_t data_type_size = data_size(column.get_type());
			size_t num_bytes_to_copy = array_size * data_type_size;
			// std::cout << "num_bytes_to_copy: " << num_bytes_to_copy << std::endl;
			memcpy(hdf5_data_ptr, tab_data_ptr, num_bytes_to_copy);

			hdf5_data_ptr += num_bytes_to_copy;
			tab_data_ptr += num_bytes_to_copy;
		} // end loop through columns
	} // end loop through rows

#else
	// std::cout << "before h5_table.write(), data.size(): " << get_data().size() << std::endl;
	h5_table.write(get_data().data(), compound_type);
	// std::cout << "after h5_table.write()" << std::endl;

#endif
}
