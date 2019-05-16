#include "../../../H5_to_Data_Type.hxx"
#include "../../../Table.hxx"
#include "../../../string_to_Data_Type.hxx"
#include "../../HDF5_Attribute.hxx"
#include "../../HDF5_Column.hxx"

namespace tablator {
bool is_columns_valid(H5::VarLenType &columns);

std::vector<Column> read_column_metadata(const H5::H5Location &dataset,
                                         const std::string &section) {
    /// Returns an empty result of the types do not match exactly.
    std::vector<Column> result;
    if (!dataset.attrExists(section)) {
        return result;
    }
    auto attribute = dataset.openAttribute(section);
    if (attribute.getTypeClass() != H5T_VLEN) {
        return result;
    }
    H5::VarLenType columns = attribute.getVarLenType();
    if (!is_columns_valid(columns)) {
        return result;
    }

    hvl_t hdf5_columns;
    attribute.read(columns, &hdf5_columns);
    for (size_t column = 0; column < hdf5_columns.len; ++column) {
        HDF5_Column &hdf5_column =
                reinterpret_cast<HDF5_Column *>(hdf5_columns.p)[column];

        std::map<std::string, std::string> attributes;
        HDF5_Field_Properties &hdf5_field_properties(hdf5_column.field_properties);
        hvl_t &hdf5_attributes(hdf5_field_properties.attributes);
        for (size_t attribute = 0; attribute < hdf5_attributes.len; ++attribute) {
            HDF5_Attribute &a(
                    reinterpret_cast<HDF5_Attribute *>(hdf5_attributes.p)[attribute]);
            const std::string name(a.name);
            if (attributes.find(name) != attributes.end()) {
                throw std::runtime_error("In " + section + " in column " +
                                         hdf5_column.name + ": duplicate attribute " +
                                         name);
            }
            attributes.insert(std::make_pair(name, std::string(a.value)));
        }

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

        std::vector<std::pair<std::string, std::string> > links;
        hvl_t &hdf5_links(hdf5_column.field_properties.links);
        for (size_t link = 0; link < hdf5_links.len; ++link) {
            HDF5_Attribute &a(reinterpret_cast<HDF5_Attribute *>(hdf5_links.p)[link]);
            links.emplace_back(std::string(a.name), std::string(a.value));
        }

        Field_Properties field_properties(hdf5_field_properties.description, attributes,
                                          values, links);
        result.emplace_back(hdf5_column.name, string_to_Data_Type(hdf5_column.type),
                            hdf5_column.array_size, field_properties);
    }
    return result;
}
}  // namespace tablator
