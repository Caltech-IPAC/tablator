#include "../../ptree_readers.hxx"

#include "../../Group_Element.hxx"
#include "../../Resource_Element.hxx"
#include "../../Table_Element.hxx"
#include "../../Utils/Table_Utils.hxx"

// FIXME: Refactor to reduce cyclomatic complexity.

tablator::Resource_Element tablator::ptree_readers::read_resource_element(
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

    std::vector<std::pair<std::string, Property>> labeled_properties;
    while (child != end && child->first == INFO) {
        labeled_properties.emplace_back(
                std::make_pair(child->first, read_property(child->second)));
        ++child;
    }

    std::vector<Field> params;
    std::vector<Group_Element> group_elements;
    while (child != end) {
        if ((child->first == COOSYS) || (child->first == TIMESYS)) {
            labeled_properties.emplace_back(
                    std::make_pair(child->first, read_property(child->second)));
        } else if (child->first == PARAM) {
            params.emplace_back(read_field(child->second).get_field());
        } else if (child->first == GROUP) {
            group_elements.emplace_back(read_group_element(child->second));
        } else if (child->first == INFO) {
            // JTODO INFO doesn't belong here unless middle is missing and this is a
            // trailer
            throw std::runtime_error("Resource_Element, found unexpected INFO element");
        } else {
            break;
        }
        ++child;
    }

    while (child != end) {
        if (child->first == LINK) {
            auto curr_attributes = extract_attributes(child->second);
            labeled_properties.emplace_back(
                    std::make_pair(child->first, Property(curr_attributes)));
        } else if (child->first == TABLE) {
            break;
        } else {
            throw std::runtime_error(
                    "Resource_Element, expected TABLE or LINK but found " +
                    child->first);
        }
        ++child;
    }

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
    while (child != end) {
        if (child->first == INFO) {
            trailing_info_list.emplace_back(read_property(child->second));
        } else {
            throw std::runtime_error(
                    std::string("Expected INFO tag, if anything, but found ") +
                    child->first);
        }
        ++child;
    }

    return tablator::Resource_Element::Builder(table_elements)
            .add_attributes(top_attributes)
            .add_description(description)
            .add_labeled_properties(labeled_properties)
            .add_trailing_info_list(trailing_info_list)
            .add_group_elements(group_elements)
            .add_params(params)
            .build();
}