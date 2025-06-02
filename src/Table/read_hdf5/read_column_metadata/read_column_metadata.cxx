#include "../../../Table.hxx"

#include "../../../H5_to_Data_Type.hxx"
#include "../../../Utils/Vector_Utils.hxx"
#include "../../../string_to_Data_Type.hxx"
#include "../../HDF5_Attribute.hxx"
#include "../../HDF5_Column.hxx"

namespace tablator {
bool is_columns_valid(H5::VarLenType &columns);

// JTODO Rename to read_field_metadata.cxx when renaming Column.hxx to Field.hxx.

std::vector<Column> read_column_metadata(const H5::H5Location &dataset,
                                         const std::string &section) {
    /// Returns an empty result if the types do not match exactly.
    std::vector<Column> result;
    if (!dataset.attrExists(section)) {
	// std::cout << "read_column_metadata(), early exit I" << std::endl;
        return result;
    }
    auto attribute = dataset.openAttribute(section);
    if (attribute.getTypeClass() != H5T_VLEN) {
	// std::cout << "read_column_metadata(), early exit II" << std::endl;
        return result;
    }
	// std::cout << "read_column_metadata(), after early exits" << std::endl;
	// JTODO rename
    H5::VarLenType columns = attribute.getVarLenType();
    if (!is_columns_valid(columns)) {
        return result;
    }
	// std::cout << "read_column_metadata(), after !is_columns_valid()" << std::endl;

    hvl_t hdf5_columns;
    attribute.read(columns, &hdf5_columns);

	// std::cout << "after attribute.read()" << std::endl;
    for (size_t col_idx = 0; col_idx < hdf5_columns.len; ++col_idx) {
	  // std::cout << "top of attribute.read() loop, col_idx: " << col_idx << std::endl;
	  HDF5_Column &hdf5_column =
                reinterpret_cast<HDF5_Column *>(hdf5_columns.p)[col_idx];

        ATTRIBUTES attributes;
        HDF5_Field_Properties &hdf5_field_properties(hdf5_column.field_properties);
        hvl_t &hdf5_attributes(hdf5_field_properties.attributes);
        for (size_t attribute = 0; attribute < hdf5_attributes.len; ++attribute) {
            HDF5_Attribute &a(
                    reinterpret_cast<HDF5_Attribute *>(hdf5_attributes.p)[attribute]);
            const std::string name(a.name);
            if (attributes.find(name) != attributes.end()) {
                throw std::runtime_error("In " + section + " in col_idx " +
                                         hdf5_column.name + ": duplicate attribute " +
                                         name);
            }
            attributes.insert(std::make_pair(name, std::string(a.value)));
        }
		// std::cout << "after attributes, size: " << attributes.size() << std::endl;
        HDF5_Values &hdf5_values(hdf5_field_properties.values);
        std::vector<Option> options;
        hvl_t &hdf5_options(hdf5_values.options);
        for (size_t option = 0; option < hdf5_options.len; ++option) {
            HDF5_Attribute &o(
                    reinterpret_cast<HDF5_Attribute *>(hdf5_options.p)[option]);
            options.emplace_back(o.name, o.value);
        }

        Min_Max min(hdf5_values.min.value, hdf5_values.min.inclusive),
                max(hdf5_values.max.value, hdf5_values.max.inclusive);
        Values values(min, max, hdf5_values.ID, hdf5_values.type, hdf5_values.null,
                      hdf5_values.ref, options);

        std::vector<STRING_PAIR> links;
        hvl_t &hdf5_links(hdf5_column.field_properties.links);
        for (size_t idx = 0; idx < hdf5_links.len; ++idx) {
            HDF5_Attribute &a(reinterpret_cast<HDF5_Attribute *>(hdf5_links.p)[idx]);
            links.emplace_back(std::string(a.name), std::string(a.value));
        }

        Field_Properties field_properties =
                Field_Properties::Builder()
                        .add_description(hdf5_field_properties.description)
                        .add_attributes(attributes)
                        .add_values(values)
                        .add_hdf5_links(links)
                        .build();

        result.emplace_back(hdf5_column.name, string_to_Data_Type(hdf5_column.type),
                            hdf5_column.array_size, field_properties, hdf5_column.dynamic_array_flag);
		// std::cout << "bottom of loop, result.size(): " << result.size() << std::endl;
    }
		// std::cout << "read_column_metadata(), return  result.size(): " << result.size() << std::endl;
    return result;
}
}  // namespace tablator
