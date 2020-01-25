#include "../../Table.hxx"

#include "../../Common.hxx"
#include "../../H5_to_Data_Type.hxx"
#include "../HDF5_Attribute.hxx"
#include "../HDF5_Property.hxx"

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
    set_resource_element_params(read_column_metadata(resource, PARAM));
    H5::DataSet dataset = resource.openDataSet(resource.getObjnameByIdx(0).c_str());

    auto &comments = get_comments();
    if (dataset.attrExists(DESCRIPTION)) {
        auto description = dataset.openAttribute(DESCRIPTION);
        if (description.getTypeClass() == H5T_STRING) {
            comments.resize(1);
            description.read(description.getDataType(), comments[0]);
        }
    }
    set_table_element_params(read_column_metadata(dataset, PARAM));
    set_labeled_properties(read_metadata(dataset));

    auto column_metadata(read_column_metadata(dataset, FIELD));
    // FIXME: This does not handle field_properties
    // FIXME: This assumes that the first column is null_bitfield_flags
    auto compound = dataset.getCompType();
    if (column_metadata.size() != static_cast<size_t>(compound.getNmembers())) {
        throw std::runtime_error(
                "Inconsistent number of fields.  The 'FIELD' metadata lists " +
                std::to_string(column_metadata.size()) + " but the dataset has " +
                std::to_string(compound.getNmembers()));
    }

    auto &columns = get_columns();
    auto &offsets = get_offsets();
    auto &data = get_data();

    for (int i = 0; i < compound.getNmembers(); ++i) {
        H5::DataType datatype(compound.getMemberDataType(i));
        std::string name(compound.getMemberName(i));
        if (datatype.getClass() == H5T_STRING) {
            append_column(columns, offsets, name, Data_Type::CHAR, datatype.getSize(),
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
            append_column(columns, offsets, name, H5_to_Data_Type(datatype), ndims,
                          column_metadata[i].get_field_properties());
        } else {
            append_column(columns, offsets, name, H5_to_Data_Type(datatype), 1,
                          column_metadata[i].get_field_properties());
        }
    }
    data.resize(row_size() * dataset.getSpace().getSimpleExtentNpoints());
    dataset.read(data.data(), compound);
}
