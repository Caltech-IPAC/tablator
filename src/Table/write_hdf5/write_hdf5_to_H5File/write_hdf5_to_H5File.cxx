#include <array>

#include "../../../Data_Type_to_H5.hxx"
#include "../../../Table.hxx"

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
    write_hdf5_columns(resource_params, "PARAM", group);

    std::array<hsize_t, 1> dims = {{num_rows()}};
    H5::DataSpace dataspace(dims.size(), dims.data());

    std::vector<H5::StrType> string_types;
    std::vector<H5::ArrayType> array_types;
    H5::CompType compound_type(row_size());
    std::vector<std::string> unique_names;
    unique_names.reserve(columns.size());
    for (auto &column : columns) {
        unique_names.push_back(column.name);
    }
    std::sort(unique_names.begin(), unique_names.end());
    if (std::unique(unique_names.begin(), unique_names.end()) != unique_names.end()) {
        throw std::runtime_error(
                "Duplicate column names are not "
                "allowed in HDF5 tables");
    }
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].type == Data_Type::CHAR) {
            string_types.emplace_back(0, columns[i].array_size);
            compound_type.insertMember(columns[i].name, offsets[i],
                                       *string_types.rbegin());
        } else if (columns[i].array_size != 1) {
            const hsize_t hsize(columns[i].array_size);
            array_types.emplace_back(Data_Type_to_H5(columns[i].type), 1, &hsize);
            compound_type.insertMember(columns[i].name, offsets[i],
                                       *array_types.rbegin());
        } else {
            compound_type.insertMember(columns[i].name, offsets[i],
                                       Data_Type_to_H5(columns[i].type));
        }
    }
    // FIXME: This needs to be generalized for multiple tables
    H5::DataSet h5_table{group.createDataSet("TABLE_0", compound_type, dataspace)};
    if (!comments.empty()) {
        std::string description;
        for (auto &c : comments) description += c + '\n';
        if (!description.empty()) {
            description.resize(description.size() - 1);
            H5::StrType str_type(0, description.size());
            H5::DataSpace attribute_space(H5S_SCALAR);
            H5::Attribute attribute =
                    h5_table.createAttribute("DESCRIPTION", str_type, attribute_space);
            attribute.write(str_type, description.c_str());
        }
    }
    write_hdf5_attributes(h5_table);
    write_hdf5_columns(columns, "FIELD", h5_table);
    write_hdf5_columns(table_params, "PARAM", h5_table);
    h5_table.write(data.data(), compound_type);
}
