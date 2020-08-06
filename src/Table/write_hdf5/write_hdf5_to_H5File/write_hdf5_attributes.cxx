#include <vector>

#include "../../../Table.hxx"
#include "../../HDF5_Attribute.hxx"
#include "../../HDF5_Property.hxx"

/// This gets a bit complicated because there are variable length
/// arrays inside variable length arrays.

void tablator::Table::write_hdf5_attributes(H5::DataSet &table) const {
    H5::StrType hdf5_string(0, H5T_VARIABLE);
    H5::CompType hdf5_attribute_type(2 * hdf5_string.getSize());
    hdf5_attribute_type.insertMember("name", 0, hdf5_string);
    hdf5_attribute_type.insertMember("value", hdf5_string.getSize(), hdf5_string);
    H5::VarLenType hdf5_attributes_type(&hdf5_attribute_type);

    H5::CompType hdf5_property_type(sizeof(HDF5_Property));
    hdf5_property_type.insertMember("name", HOFFSET(HDF5_Property, name), hdf5_string);
    hdf5_property_type.insertMember("value", HOFFSET(HDF5_Property, value),
                                    hdf5_string);
    hdf5_property_type.insertMember("attributes", HOFFSET(HDF5_Property, attributes),
                                    hdf5_attributes_type);
    H5::VarLenType hdf5_properties_type(&hdf5_property_type);

    // FIXME: This uses .c_str() of std::string, which is not guaranteed
    // to hang around.
    std::vector<std::vector<const char *> > strings;
    std::vector<HDF5_Property> hdf5_properties;

    // Combine and write properties/attributes/trailing_info_lists for
    // all levels at which they are defined.
    // JTODO function to do all the combining
    auto combined_labeled_properties = combine_labeled_properties_all_levels();
    const auto combined_labeled_trailing_info_lists =
            combine_trailing_info_lists_all_levels();
    const auto combined_labeled_attributes = combine_attributes_all_levels();

    combined_labeled_properties.insert(combined_labeled_properties.end(),
                                       combined_labeled_trailing_info_lists.begin(),
                                       combined_labeled_trailing_info_lists.end());
    combined_labeled_properties.insert(combined_labeled_properties.end(),
                                       combined_labeled_attributes.begin(),
                                       combined_labeled_attributes.end());

    for (auto &label_and_prop : combined_labeled_properties) {
        const auto &label = label_and_prop.first;
        const auto &prop = label_and_prop.second;

        if (!prop.empty()) {
            strings.emplace_back();
            std::vector<const char *> &sub_vector = *strings.rbegin();
            for (auto &att : prop.get_attributes()) {
                sub_vector.push_back(att.first.c_str());
                sub_vector.push_back(att.second.c_str());
            }

            hvl_t hvl_atts;
            hvl_atts.len = sub_vector.size() / 2;
            hvl_atts.p = sub_vector.data();
            hdf5_properties.emplace_back(label.c_str(), prop.get_value().c_str(),
                                         hvl_atts);
        }
    }


    const std::string empty_string = "";
    const ATTRIBUTES &resource_element_attributes =
            get_results_resource_element().get_attributes();
    if (!resource_element_attributes.empty()) {
        strings.emplace_back();
        std::vector<const char *> &sub_vector = *strings.rbegin();
        for (const auto &att : resource_element_attributes) {
            const std::string new_name = XMLATTR_DOT + att.first;
            sub_vector.push_back(att.first.c_str());
            sub_vector.push_back(att.second.c_str());


            hvl_t hvl_atts;
            hvl_atts.len = sub_vector.size() / 2;
            hvl_atts.p = sub_vector.data();
            hdf5_properties.emplace_back("VOTABLE.RESOURCE.ATTR_MARKER",
                                         empty_string.c_str(), hvl_atts);
        }
    }


    hvl_t hdf5_props;
    hdf5_props.len = hdf5_properties.size();
    hdf5_props.p = hdf5_properties.data();

    H5::DataSpace property_space(H5S_SCALAR);
    H5::Attribute table_attribute =
            table.createAttribute("METADATA", hdf5_properties_type, property_space);
    table_attribute.write(hdf5_properties_type, &hdf5_props);
}
