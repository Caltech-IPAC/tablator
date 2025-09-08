#include "../../../ptree_readers.hxx"

#include <boost/algorithm/string/predicate.hpp>

#include "../../../Property.hxx"
#include "../../../Table_Element.hxx"
#include "../../../Utils/Null_Utils.hxx"
#include "../../../Utils/Vector_Utils.hxx"
#include "../../Utils.hxx"


namespace {

// Helper function which handles the part of the property_tree corresponding to
// the section of a VOTable TABLE element preceding its DATA element per the IVOA spec.

boost::property_tree::ptree::const_iterator load_pre_data_section(
        std::vector<tablator::Field> &fields, std::vector<tablator::Field> &params,
        std::vector<tablator::Group_Element> &group_elements,
        bool &has_dynamic_array_column,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end) {
    has_dynamic_array_column = false;
    boost::property_tree::ptree::const_iterator &iter = start;
    while (iter != end) {
        if (iter->first == tablator::FIELD) {
            has_dynamic_array_column |=
                    tablator::ptree_readers::load_field_singleton(fields, iter->second);
        } else if (iter->first == tablator::FIELD_ARRAY) {
            has_dynamic_array_column |=
                    tablator::ptree_readers::load_field_array(fields, iter->second);
        } else if (iter->first == tablator::PARAM) {
            tablator::ptree_readers::load_field_singleton(params, iter->second);
        } else if (iter->first == tablator::PARAM_ARRAY) {
            tablator::ptree_readers::load_field_array(params, iter->second);

        } else if (iter->first == tablator::GROUP) {
            tablator::ptree_readers::load_group_element_singleton(group_elements,
                                                                  iter->second);
        } else if (iter->first == tablator::GROUP_ARRAY) {
            tablator::ptree_readers::load_group_element_array(group_elements,
                                                              iter->second);

        } else if ((iter->first == tablator::DATA) || (iter->first == tablator::INFO) ||
                   (iter->first == tablator::INFO_ARRAY)) {
            break;
        } else {
            throw std::runtime_error(
                    "In table element, expected FIELD, PARAM, GROUP, DATA, or INFO, "
                    "but found " +
                    iter->first);
        }
        ++iter;
    }
    return iter;
}

}  // namespace


namespace tablator {
namespace ptree_readers {

//==============================================

tablator::Table_Element read_table_element(
        const boost::property_tree::ptree &table_tree) {
    auto child = table_tree.begin();
    auto end = table_tree.end();

    const auto attributes = extract_attributes(table_tree);

    while (child != end && child->first == XMLATTR) {
        ++child;
    }

    std::string description;
    if (child != end && child->first == DESCRIPTION) {
        description = child->second.get_value<std::string>();
        ++child;
    }

    std::vector<Field> fields;
    // Register null column
    fields.emplace_back(null_bitfield_flags_name, Data_Type::UINT8_LE, true,
                        Field_Properties(null_bitfield_flags_description, {}));

    std::vector<Group_Element> group_elements;
    std::vector<Field> params;
    bool record_dynamic_array_sizes_f = false;
    child = load_pre_data_section(fields, params, group_elements,
                                  record_dynamic_array_sizes_f, child, end);
    if (fields.size() < 2) {
        throw std::runtime_error("This VOTable is empty.");
    }

    std::vector<Data_Element> data_elements;
    if (child != end && child->first == DATA) {
        data_elements.emplace_back(
                read_data_element(child->second, fields, record_dynamic_array_sizes_f));
        ++child;
    }
    std::vector<Property> trailing_info_list;
    child = read_trailing_info_section(trailing_info_list, child, end);

    return tablator::Table_Element::Builder(data_elements)
            .add_attributes(attributes)
            .add_description(description)
            .add_group_elements(group_elements)
            .add_params(params)
            .add_fields(fields)
            .add_trailing_info_list(trailing_info_list)
            .build();
}


}  // namespace ptree_readers
}  // namespace tablator
