#include "../ptree_readers.hxx"

#include "../Table.hxx"
#include "Utils.hxx"

// This only parses VOTable v1.3.  // JTODO 1.4 now?

namespace {

bool load_resource_element_singleton(
        std::vector<tablator::Resource_Element> &resource_elements,
        const boost::property_tree::ptree &node,
        bool &already_loaded_results_resource) {
    bool is_results_resource = false;
    resource_elements.emplace_back(
            tablator::ptree_readers::read_resource_element(node, is_results_resource));

    if (is_results_resource) {
        if (already_loaded_results_resource) {
            throw std::runtime_error("TABLE element is allowed in only one RESOURCE.");
        }
        already_loaded_results_resource = true;
    }
    return is_results_resource;
}

bool load_resource_element_array(
        std::vector<tablator::Resource_Element> &resource_elements,
        const boost::property_tree::ptree &array_tree,
        bool &already_loaded_results_resource, size_t start_resource_idx,
        size_t &results_resource_idx) {
    bool contains_results_resource = false;
    size_t curr_resource_idx = start_resource_idx;
    for (const auto &elt : array_tree) {
        bool is_results_resource = load_resource_element_singleton(
                resource_elements, elt.second, already_loaded_results_resource);
        if (is_results_resource) {
            results_resource_idx = curr_resource_idx;
            contains_results_resource = true;
        }
        ++curr_resource_idx;
    }
    return contains_results_resource;
}


// Helper function which handles the part of the property_tree corresponding to
// the section of a VOTable preceding the RESOURCE element per the IVOA spec.

boost::property_tree::ptree::const_iterator populate_pre_resource_section(
        tablator::Labeled_Properties &labeled_properties,
        std::vector<tablator::Field> &params,
        std::vector<tablator::Group_Element> &group_elements,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end) {
    boost::property_tree::ptree::const_iterator &iter = start;

    while (iter != end && !boost::starts_with(iter->first, tablator::RESOURCE)) {
        if ((iter->first == tablator::COOSYS) || (iter->first == tablator::TIMESYS) ||
            (iter->first == tablator::INFO)) {
            tablator::ptree_readers::load_labeled_properties_singleton(
                    labeled_properties, tablator::INFO, iter->second);
        } else if (iter->first == tablator::INFO_ARRAY) {
            tablator::ptree_readers::load_labeled_properties_array(
                    labeled_properties, tablator::INFO, iter->second);

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

        } else {
            throw std::runtime_error(
                    "In VOTABLE, expected COOSYS, TIMESYS, GROUP, "
                    "PARAM, INFO, RESOURCE, or a comment, but got: " +
                    iter->first);
        }
        ++iter;
    }
    return iter;
}

}  // namespace

//==============================================================

void tablator::ptree_readers::read_property_tree_as_votable(
        tablator::Table &table, const boost::property_tree::ptree &tree) {
    const auto votable = tree.get_child(VOTABLE);
    auto child = votable.begin();
    auto end = votable.end();

    Property votable_property("");
    while (child != end && child->first == XMLATTR) {
        // We'll re-create this section if/when we write the table in IPAC format.
        for (auto &attribute : child->second) {
            if (!(attribute.first == "version" || attribute.first == "xmlns:xsi" ||
                  attribute.first == "xmlns" || attribute.first == "xmlns:stc" ||
                  attribute.first == "xsi:schemaLocation" ||
                  attribute.first == "xsi:noNamespaceSchemaLocation"))
                votable_property.add_attribute(
                        attribute.first, attribute.second.get_value<std::string>());
        }
        ++child;
    }
    if (!votable_property.empty()) {
        table.add_labeled_property(VOTABLE, votable_property);
    }

    if (child != end && child->first == DESCRIPTION) {
        table.set_description(child->second.get_value<std::string>());
        ++child;
    }
    if (child != end && child->first == DEFINITIONS) {
        // Deliberately ignore DEFINITIONS.  They are duplicated by the
        // information in RESOURCE and deprecated since version 1.1.
        ++child;
    }

    auto &labeled_properties = table.get_labeled_properties();
    auto &params = table.get_params();
    auto &group_elements = table.get_group_elements();

    child = populate_pre_resource_section(labeled_properties, params, group_elements,
                                          child, end);
    if (child == end) {
        throw std::runtime_error("Missing RESOURCE in VOTABLE");
    }

    // read resources
    auto &resource_elements = table.get_resource_elements();
    bool found_results_resource = false;
    size_t curr_resource_idx = 0;
    size_t results_resource_idx = 0;

    while (child != end &&
           (child->first == RESOURCE || child->first == RESOURCE_ARRAY)) {
        if (child->first == RESOURCE) {
            bool is_results_resource = load_resource_element_singleton(
                    resource_elements, child->second, found_results_resource);
            if (is_results_resource) {
                table.set_results_resource_idx(curr_resource_idx);
            }
        } else {
            bool contains_results_resource = load_resource_element_array(
                    resource_elements, child->second, found_results_resource,
                    curr_resource_idx, results_resource_idx);
            if (contains_results_resource) {
                table.set_results_resource_idx(results_resource_idx);
            }
        }
        ++curr_resource_idx;
        ++child;
    }
    if (!found_results_resource) {
        throw std::runtime_error("Missing results RESOURCE.");
    }

    std::vector<Property> &trailing_info_list = table.get_trailing_info_list();
    child = read_trailing_info_section(trailing_info_list, child, end);
}
