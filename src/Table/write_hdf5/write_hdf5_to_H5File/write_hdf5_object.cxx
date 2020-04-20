#include "../../../Column.hxx"
#include "../../../Common.hxx"
#include "../../../Field_Properties.hxx"
#include "../../HDF5_Attribute.hxx"
#include "../../HDF5_Column.hxx"
#include "../../HDF5_Property.hxx"

#include <deque>
#include <vector>
// FIXME: This does not handle recursive OPTION's inside an OPTION.
// If I try to define an OPTION array without adding it to theh OPTION
// type first, then the OPTION inside the array does not have an
// array.

namespace tablator {
hvl_t make_option_array(const std::vector<Option> &options,
                        std::deque<std::vector<HDF5_Attribute> > &option_arrays) {
    option_arrays.emplace_back();
    std::vector<HDF5_Attribute> &option_vector = *option_arrays.rbegin();
    for (auto &option : options) {
        option_vector.emplace_back(option.name.c_str(), option.value.c_str());
    }
    hvl_t hdf5_options = {option_vector.size(), option_vector.data()};
    return hdf5_options;
}

void write_hdf5_object(H5::H5Object &h5_object, const std::vector<Field> &fields,
                       const std::string &field_name) {
    if (fields.empty()) {
        return;
    }
    H5::StrType hdf5_string(0, H5T_VARIABLE);

    H5::CompType hdf5_option(sizeof(HDF5_Attribute));
    hdf5_option.insertMember(ATTR_NAME, HOFFSET(HDF5_Attribute, name), hdf5_string);
    hdf5_option.insertMember(ATTR_VALUE, HOFFSET(HDF5_Attribute, value), hdf5_string);

    H5::VarLenType hdf5_option_array(&hdf5_option);

    H5::CompType hdf5_min_max(sizeof(HDF5_Min_Max));
    hdf5_min_max.insertMember("value", HOFFSET(HDF5_Min_Max, value), hdf5_string);
    hdf5_min_max.insertMember("inclusive", HOFFSET(HDF5_Min_Max, inclusive),
                              H5::PredType::STD_I8LE);

    H5::CompType hdf5_values(sizeof(HDF5_Values));
    hdf5_values.insertMember("min", HOFFSET(HDF5_Values, min), hdf5_min_max);
    hdf5_values.insertMember("max", HOFFSET(HDF5_Values, max), hdf5_min_max);
    hdf5_values.insertMember("ID", HOFFSET(HDF5_Values, ID), hdf5_string);
    hdf5_values.insertMember("type", HOFFSET(HDF5_Values, type), hdf5_string);
    hdf5_values.insertMember("null", HOFFSET(HDF5_Values, null), hdf5_string);
    hdf5_values.insertMember("ref", HOFFSET(HDF5_Values, ref), hdf5_string);
    hdf5_values.insertMember("options", HOFFSET(HDF5_Values, options),
                             hdf5_option_array);

    H5::CompType hdf5_attribute(2 * hdf5_string.getSize());
    hdf5_attribute.insertMember(ATTR_NAME, 0, hdf5_string);
    hdf5_attribute.insertMember(ATTR_VALUE, hdf5_string.getSize(), hdf5_string);

    H5::VarLenType hdf5_attribute_array(&hdf5_attribute);

    H5::CompType hdf5_field_properties(sizeof(HDF5_Field_Properties));
    hdf5_field_properties.insertMember(Field_Properties::FP_DESCRIPTION,
                                       HOFFSET(HDF5_Field_Properties, description),
                                       hdf5_string);
    hdf5_field_properties.insertMember(Field_Properties::FP_ATTRIBUTES,
                                       HOFFSET(HDF5_Field_Properties, attributes),
                                       hdf5_attribute_array);
    hdf5_field_properties.insertMember(Field_Properties::FP_LINKS,
                                       HOFFSET(HDF5_Field_Properties, links),
                                       hdf5_attribute_array);
    hdf5_field_properties.insertMember(Field_Properties::FP_VALUES,
                                       HOFFSET(HDF5_Field_Properties, values),
                                       hdf5_values);

    H5::CompType hdf5_column(sizeof(HDF5_Column));
    hdf5_column.insertMember(Column::COL_NAME, HOFFSET(HDF5_Column, name), hdf5_string);
    hdf5_column.insertMember(Column::COL_TYPE, HOFFSET(HDF5_Column, type), hdf5_string);
    hdf5_column.insertMember(Column::COL_ARRAY_SIZE, HOFFSET(HDF5_Column, array_size),
                             H5::PredType::STD_U64LE);
    hdf5_column.insertMember(Column::COL_FIELD_PROPERTIES,
                             HOFFSET(HDF5_Column, field_properties),
                             hdf5_field_properties);

    H5::VarLenType hdf5_columns_type(&hdf5_column);

    std::deque<std::vector<const char *> > strings;
    std::vector<HDF5_Column> hdf5_columns;
    std::deque<std::string> type_strings;  /// Use a deque to avoid
                                           /// invalidating iterators
    std::deque<std::vector<HDF5_Attribute> > option_arrays;
    for (auto &field : fields) {
        const Field_Properties &field_properties(field.get_field_properties());

        strings.emplace_back();
        std::vector<const char *> &attributes_vector = *strings.rbegin();
        for (auto &a : field_properties.get_attributes()) {
            attributes_vector.push_back(a.first.c_str());
            attributes_vector.push_back(a.second.c_str());
        }
        hvl_t hdf5_attributes = {attributes_vector.size() / 2,
                                 attributes_vector.data()};

        strings.emplace_back();
        std::vector<const char *> &links_vector = *strings.rbegin();
        for (auto &a : field_properties.get_hdf5_links()) {
            links_vector.push_back(a.first.c_str());
            links_vector.push_back(a.second.c_str());
        }
        hvl_t hdf5_links = {links_vector.size() / 2, links_vector.data()};

        const Values &values(field_properties.get_values());

        HDF5_Min_Max hdf5_min = {values.min.value.c_str(), values.min.inclusive},
                     hdf5_max = {values.max.value.c_str(), values.max.inclusive};

        option_arrays.emplace_back();
        std::vector<HDF5_Attribute> &option_vector = *option_arrays.rbegin();
        for (auto &option : values.options) {
            option_vector.emplace_back(option.name.c_str(), option.value.c_str());
        }
        hvl_t hdf5_options = {option_vector.size(), option_vector.data()};

        HDF5_Values hdf5_values = {hdf5_min,
                                   hdf5_max,
                                   values.ID.c_str(),
                                   values.type.c_str(),
                                   values.null.c_str(),
                                   values.ref.c_str(),
                                   hdf5_options};

        HDF5_Field_Properties hdf5_field_properties = {
                field_properties.get_description().c_str(), hdf5_attributes, hdf5_links,
                hdf5_values};
        type_strings.push_back(to_string(field.get_type()));
        hdf5_columns.emplace_back(field.get_name().c_str(),
                                  type_strings.rbegin()->c_str(),
                                  field.get_array_size(), hdf5_field_properties);
    }

    hvl_t H5_columns = {hdf5_columns.size(), hdf5_columns.data()};

    H5::DataSpace column_space(H5S_SCALAR);
    H5::Attribute h5_object_attribute =
            h5_object.createAttribute(field_name, hdf5_columns_type, column_space);
    h5_object_attribute.write(hdf5_columns_type, &H5_columns);
}
}  // namespace tablator
