#include "../../ptree_readers.hxx"

#include "../../Group_Element.hxx"
#include "../../Resource_Element.hxx"
#include "../../Table_Element.hxx"
#include "../Utils.hxx"

// FIXME: Refactor to reduce cyclomatic complexity.

namespace {


boost::property_tree::ptree::const_iterator read_top_info_section(
        tablator::Labeled_Properties &labeled_properties,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end) {
    boost::property_tree::ptree::const_iterator &iter = start;
    while (iter != end) {
        if (iter->first == tablator::INFO) {
            tablator::ptree_readers::load_labeled_properties_singleton(
                    labeled_properties, tablator::INFO, iter->second);
        } else if (iter->first == tablator::INFO_ARRAY) {
            tablator::ptree_readers::load_labeled_properties_array(
                    labeled_properties, tablator::INFO, iter->second);
        } else {
            break;
        }
        ++iter;
    }
    return iter;
}


// Helper function which handles the part of the property_tree corresponding to
// the section of a VOTable preceding the LINK element(s) per the IVOA spec.

boost::property_tree::ptree::const_iterator read_pre_links_section(
        tablator::Labeled_Properties &labeled_properties,
        std::vector<tablator::Field> &params,
        std::vector<tablator::Group_Element> &group_elements,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end) {
    boost::property_tree::ptree::const_iterator &iter = start;
    while (iter != end) {
        if ((iter->first == tablator::COOSYS) || (iter->first == tablator::TIMESYS)) {
            // There should only be one of each.
            tablator::ptree_readers::load_labeled_properties_singleton(
                    labeled_properties, iter->first, iter->second);

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

        } else if (boost::starts_with(iter->first, tablator::INFO)) {
            // JTODO INFO might possibly be allowable here if there is no
            // table and this is a trailer
            throw std::runtime_error("Resource_Element, found unexpected INFO element");
        } else {
            break;
        }
        ++iter;
    }
    return iter;
}


}  // namespace


namespace tablator {
namespace ptree_readers {
//============================================================================

tablator::Resource_Element read_resource_element(
        const boost::property_tree::ptree &resource_tree, bool &is_results_resource) {
    auto child = resource_tree.begin();
    auto end = resource_tree.end();

    const auto top_attributes = extract_attributes(resource_tree);

    // JTODO Assume XMLATTRs are all at front?
    while (child != end && child->first == XMLATTR) {
        ++child;
    }

    std::string description;
    if (child != end && child->first == DESCRIPTION) {
        description = child->second.get_value<std::string>();
        ++child;
    }

    Labeled_Properties labeled_properties;
    child = read_top_info_section(labeled_properties, child, end);

    std::vector<Field> params;
    std::vector<Group_Element> group_elements;
    child = read_pre_links_section(labeled_properties, params, group_elements, child,
                                   end);

    child = read_links_section(labeled_properties, child, end, TABLE);

    // We only allow one (nonempty) TABLE in one RESOURCE and none in the others.
    std::vector<Table_Element> table_elements;
    while (child != end) {
        if (child->first == TABLE) {
            if (!table_elements.empty()) {
                throw std::runtime_error(
                        "Multiple TABLE elements are not implemented.");
            }
            is_results_resource = true;
            table_elements.emplace_back(read_table_element(child->second));
        } else if (child->first == INFO) {
            break;
        } else {
            throw std::runtime_error(
                    "Resource_Element, expected TABLE or INFO but found " +
                    child->first);
        }
        ++child;
    }

    std::vector<Property> trailing_info_list;
    child = read_trailing_info_section(trailing_info_list, child, end);

    return tablator::Resource_Element::Builder(table_elements)
            .add_attributes(top_attributes)
            .add_description(description)
            .add_labeled_properties(labeled_properties)
            .add_trailing_info_list(trailing_info_list)
            .add_group_elements(group_elements)
            .add_params(params)
            .build();
}

}  // namespace ptree_readers
}  // namespace tablator
