#include "../../Table.hxx"

#include "../../Common.hxx"
#include "../../H5_to_Data_Type.hxx"
#include "../HDF5_Attribute.hxx"
#include "../HDF5_Property.hxx"

// As of 08Sep25, CHAR columns are created with dynamic_array_flag true
// and all other columns are created with dynamic_array_flag false.

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

namespace tablator {
Labeled_Properties read_metadata(const H5::DataSet &dataset);

std::vector<Column> read_column_metadata(const H5::H5Object &h5_object,
                                         const std::string &section);

}  // namespace tablator

void tablator::Table::read_hdf5(const boost::filesystem::path &path) {
    H5::H5File file(path.string(), H5F_ACC_RDONLY);
    // FIXME: This needs to be generalized for multiple resources and
    // multiple tables

    H5::Group resource = file.openGroup("/RESOURCE_0");
    std::vector<tablator::Field> resource_params =
            read_column_metadata(resource, PARAM);

    H5::DataSet dataset = resource.openDataSet(resource.getObjnameByIdx(0).c_str());
    set_description(read_description(dataset, DESCRIPTION));
    const auto resource_element_description =
            read_description(dataset, RESOURCE_ELEMENT_DESCRIPTION);
    const auto table_element_description =
            read_description(dataset, TABLE_ELEMENT_DESCRIPTION);

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

    distribute_metadata(resource_element_labeled_properties,
                        resource_element_trailing_infos, resource_element_attributes,
                        table_element_trailing_infos, table_element_attributes,
                        metadata);

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

    std::vector<Column> orig_columns;

    for (int i = 0; i < compound.getNmembers(); ++i) {
        H5::DataType datatype(compound.getMemberDataType(i));
        std::string name(compound.getMemberName(i));
        if (datatype.getClass() == H5T_STRING) {
            orig_columns.emplace_back(name, Data_Type::CHAR, datatype.getSize(),
                                      column_metadata[i].get_field_properties(),
                                      column_metadata[i].get_dynamic_array_flag());
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
            orig_columns.emplace_back(
                    name, H5_to_Data_Type(datatype), ndims,
                    column_metadata[i].get_field_properties(),
                    false /* column_metadata[i].get_dynamic_array_flag() */);
        } else {
            orig_columns.emplace_back(
                    name, H5_to_Data_Type(datatype), 1,
                    column_metadata[i].get_field_properties(),
                    false /* column_metadata[i].get_dynamic_array_flag() */);
        }
    }

    size_t num_rows = dataset.getSpace().getSimpleExtentNpoints();

    Field_Framework field_framework(orig_columns, true /* got_null_bitfields_column */);
    Data_Details data_details(field_framework, num_rows);

    const auto &columns = field_framework.get_columns();
    const auto &offsets = field_framework.get_offsets();

    // Resize, don't just reserve.
    data_details.adjust_num_rows(num_rows);
    uint8_t *data_ptr_orig = data_details.get_data().data();
    dataset.read(data_ptr_orig, compound);

    std::vector<std::vector<uint32_t>> dynamic_array_sizes_by_row;
    bool got_dynamic_columns = data_details.got_dynamic_columns();


    size_t row_size = field_framework.get_row_size();
    const uint8_t *row_data_ptr = data_ptr_orig;
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx, row_data_ptr += row_size) {
        if (got_dynamic_columns) {
            dynamic_array_sizes_by_row.emplace_back();
        }

        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            const auto &column = columns[col_idx];
            bool dynamic_array_flag = column.get_dynamic_array_flag();
            size_t max_array_size = column.get_array_size();
            if (dynamic_array_flag) {
                if (column.get_type() != Data_Type::CHAR) {
                    throw std::runtime_error(
                            "HDF5 code currently allows only columns of type CHAR to "
                            "have variable size");
                }
                size_t col_offset = offsets.at(col_idx);
                dynamic_array_sizes_by_row.back().push_back(
                        std::min(max_array_size,
                                 strlen(reinterpret_cast<const char *>(row_data_ptr) +
                                        col_offset)));
            }
        }  // end loop through columns
    }      // end loop through rows

    data_details.get_dynamic_array_sizes_by_row().swap(dynamic_array_sizes_by_row);


    const auto table_element =
            Table_Element::Builder(field_framework, data_details)
                    .add_params(table_element_params)
                    .add_description(table_element_description)
                    .add_trailing_info_list(table_element_trailing_infos)
                    .add_attributes(table_element_attributes)
                    .build();

    add_resource_element(
            Resource_Element::Builder(table_element)
                    .add_params(resource_params)
                    .add_labeled_properties(resource_element_labeled_properties)
                    .add_description(resource_element_description)
                    .add_trailing_info_list(resource_element_trailing_infos)
                    .add_attributes(resource_element_attributes)
                    .build());
}
