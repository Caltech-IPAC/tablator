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

    write_hdf5_columns(get_resource_element_params(), PARAM, group);

    std::array<hsize_t, 1> dims = {{get_num_rows()}};
    H5::DataSpace dataspace(dims.size(), dims.data());

    const auto &columns = get_columns();
    const auto &offsets = get_offsets();

    std::vector<H5::StrType> string_types;
    std::vector<H5::ArrayType> array_types;
    H5::CompType compound_type(get_row_size());
    std::vector<std::string> unique_names;

    unique_names.reserve(columns.size());
    for (auto &column : columns) {
        unique_names.push_back(column.get_name());
    }

    std::sort(unique_names.begin(), unique_names.end());
    if (std::unique(unique_names.begin(), unique_names.end()) != unique_names.end()) {
        throw std::runtime_error(
                "Duplicate column names are not "
                "allowed in HDF5 tables");
    }

    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].get_type() == Data_Type::CHAR) {
            string_types.emplace_back(0, columns[i].get_array_size());
            compound_type.insertMember(columns[i].get_name(), offsets[i],
                                       *string_types.rbegin());
        } else if (columns[i].get_array_size() != 1) {
            const hsize_t hsize(columns[i].get_array_size());
            array_types.emplace_back(Data_Type_to_H5(columns[i].get_type()), 1, &hsize);
            compound_type.insertMember(columns[i].get_name(), offsets[i],
                                       *array_types.rbegin());
        } else {
            compound_type.insertMember(columns[i].get_name(), offsets[i],
                                       Data_Type_to_H5(columns[i].get_type()));
        }
    }
    // FIXME: This needs to be generalized for multiple tables
    H5::DataSet h5_table{group.createDataSet("TABLE_0", compound_type, dataspace)};
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

    write_description(h5_table, RESOURCE_ELEMENT_DESCRIPTION,
                      get_results_resource_element().get_description());
    write_description(h5_table, TABLE_ELEMENT_DESCRIPTION,
                      get_main_table_element().get_description());

    write_hdf5_attributes(h5_table);

    write_hdf5_columns(columns, FIELD, h5_table);
    // JTODO Top-level params? (Master doesn't.)

    write_hdf5_columns(get_table_element_params(), PARAM, h5_table);

    h5_table.write(get_data().data(), compound_type);
}
