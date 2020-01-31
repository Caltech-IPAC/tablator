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
std::vector<std::pair<std::string, Property> > read_metadata(
        const H5::DataSet &dataset);

std::vector<Column> read_column_metadata(const H5::H5Location &dataset,
                                         const std::string &section);


}  // namespace tablator

void tablator::Table::read_hdf5(const boost::filesystem::path &path) {
    H5::H5File file(path.string(), H5F_ACC_RDONLY);
    // FIXME: This needs to be generalized for multiple resources and
    // multiple tables
    H5::Group resource = file.openGroup("/RESOURCE_0");
    std::vector<tablator::Column> resource_params =
            read_column_metadata(resource, "PARAM");

    H5::DataSet dataset = resource.openDataSet(resource.getObjnameByIdx(0).c_str());

    set_description(read_description(dataset, DESCRIPTION));
    const auto resource_element_description =
            read_description(dataset, RESOURCE_ELEMENT_DESCRIPTION);
    const auto table_element_description =
            read_description(dataset, TABLE_ELEMENT_DESCRIPTION);

    std::vector<tablator::Column> table_element_params =
            read_column_metadata(dataset, "PARAM");

    const auto metadata = read_metadata(dataset);

    // Distribute the mishmash of labeled_properties stored as metadata
    // between assorted class members at assorted levels.
    std::vector<std::pair<std::string, Property> > resource_element_labeled_properties;
    std::vector<Property> resource_element_trailing_infos;
    ATTRIBUTES resource_element_attributes;
    std::vector<Property> table_element_trailing_infos;
    ATTRIBUTES table_element_attributes;


    distribute_metadata(resource_element_labeled_properties,
                        resource_element_trailing_infos, resource_element_attributes,
                        table_element_trailing_infos, table_element_attributes,
                        metadata);

    std::vector<tablator::Column> column_metadata =
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
            tablator::append_column(columns, offsets, name, Data_Type::CHAR,
                                    datatype.getSize(),
                                    column_metadata[i].get_field_properties());
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
            tablator::append_column(columns, offsets, name, H5_to_Data_Type(datatype),
                                    ndims, column_metadata[i].get_field_properties());
        } else {
            tablator::append_column(columns, offsets, name, H5_to_Data_Type(datatype),
                                    1, column_metadata[i].get_field_properties());
        }
    }
    std::vector<uint8_t> data;
    data.resize(tablator::row_size(offsets) *
                dataset.getSpace().getSimpleExtentNpoints());
    dataset.read(data.data(), compound);

    std::vector<Table_Element> table_elements;
    const auto table_element =
            Table_Element::Builder(columns, offsets, data)
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
