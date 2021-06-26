#include "../../../ptree_readers.hxx"

#include <boost/algorithm/string/predicate.hpp>

#include "../../../Property.hxx"
#include "../../../Table_Element.hxx"
#include "../../../Utils/Table_Utils.hxx"
#include "../../../Utils/Vector_Utils.hxx"
#include "../../Utils.hxx"

namespace {

void load_field_and_flag_singleton(
        std::vector<tablator::ptree_readers::Field_And_Flag> &field_flag_pairs,
        const boost::property_tree::ptree &node) {
    field_flag_pairs.emplace_back(tablator::ptree_readers::read_field(node));
}

void load_field_and_flag_array(
        std::vector<tablator::ptree_readers::Field_And_Flag> &field_flag_pairs,
        const boost::property_tree::ptree &array_tree) {
    for (const auto &elt : array_tree) {
        load_field_and_flag_singleton(field_flag_pairs, elt.second);
    }
}


// Helper function which handles the part of the property_tree corresponding to
// the section of a VOTable TABLE element preceding its DATA element per the IVOA spec.

boost::property_tree::ptree::const_iterator load_pre_data_section(
        std::vector<tablator::ptree_readers::Field_And_Flag> &field_flag_pairs,
        std::vector<tablator::Field> &params,
        std::vector<tablator::Group_Element> &group_elements,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end) {
    boost::property_tree::ptree::const_iterator &iter = start;
    while (iter != end) {
        if (iter->first == tablator::FIELD) {
            load_field_and_flag_singleton(field_flag_pairs, iter->second);
        } else if (iter->first == tablator::FIELD_ARRAY) {
            load_field_and_flag_array(field_flag_pairs, iter->second);

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

    std::vector<Field_And_Flag> field_flag_pairs;
    // Register null column
    field_flag_pairs.emplace_back(
            Field(null_bitfield_flags_name, Data_Type::UINT8_LE, true,
                  Field_Properties(null_bitfield_flags_description, {})),
            false);

    std::vector<Group_Element> group_elements;
    std::vector<Field> params;
    child = load_pre_data_section(field_flag_pairs, params, group_elements, child, end);

    if (field_flag_pairs.size() < 2) {
        throw std::runtime_error("This VOTable is empty.");
    }

    std::vector<Data_Element> data_elements;
    if (child != end && child->first == DATA) {
        data_elements.emplace_back(read_data_element(child->second, field_flag_pairs));
        ++child;
    }

    std::vector<Property> trailing_info_list;
    child = read_trailing_info_section(trailing_info_list, child, end);

    // wrap up
    std::vector<Field> fields;
    std::transform(field_flag_pairs.begin(), field_flag_pairs.end(),
                   std::back_inserter(fields),
                   [&](const Field_And_Flag &field_flag) -> Field {
                       return field_flag.get_field();
                   });

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
