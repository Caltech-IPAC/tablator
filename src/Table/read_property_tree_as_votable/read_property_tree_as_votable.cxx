#include "../../Table.hxx"
#include "skip_xml_comments.hxx"

/// This only parses VOTable v1.3.

void tablator::Table::read_property_tree_as_votable(
        const boost::property_tree::ptree &tree) {
    const auto votable = tree.get_child("VOTABLE");
    auto child = votable.begin();
    auto end = votable.end();

    Property votable_property("");
    while (child != end && child->first == "<xmlattr>") {
        // We'll re-create this section if/when we write the table in IPAC format.
        for (auto &attribute : child->second)
            if (!(attribute.first == "version" || attribute.first == "xmlns:xsi" ||
                  attribute.first == "xmlns" || attribute.first == "xmlns:stc" ||
                  attribute.first == "xsi:schemaLocation" ||
                  attribute.first == "xsi:noNamespaceSchemaLocation"))
                votable_property.attributes.insert(std::make_pair(
                        attribute.first, attribute.second.get_value<std::string>()));
        ++child;
    }
    if (!votable_property.value.empty() || !votable_property.attributes.empty()) {
        properties.emplace_back("VOTABLE", votable_property);
    }

    child = skip_xml_comments(child, end);
    if (child != end && child->first == "DESCRIPTION") {
        comments.emplace_back(child->second.get_value<std::string>());
        ++child;
    }
    child = skip_xml_comments(child, end);
    if (child != end && child->first == "DEFINITIONS") {
        /// Deliberately ignore DEFINITIONS.  They are duplicated by the
        /// information in RESOURCE and deprecated since version 1.1.
        ++child;
    }
    child = skip_xml_comments(child, end);
    while (child != end && child->first != "RESOURCE") {
        if (child->first == "COOSYS" || child->first == "PARAM" ||
            child->first == "INFO") {
            read_node_and_attributes(child);
        } else if (child->first == "GROUP") {
            // FIXME: Implement groups
        } else {
            throw std::runtime_error(
                    "In VOTABLE, expected COOSYS, GROUP, "
                    "PARAM, INFO, RESOURCE, or a comment, but got: " +
                    child->first);
        }
        ++child;
        child = skip_xml_comments(child, end);
    }
    if (child == end) {
        throw std::runtime_error("Missing RESOURCE in VOTABLE");
    } else {
        read_resource(child->second);
    }
    ++child;
    child = skip_xml_comments(child, end);
    if (child != end && child->first == "INFO") {
        read_node_and_attributes(child);
        ++child;
    }
    child = skip_xml_comments(child, end);
    if (child != end) {
        throw std::runtime_error("Unexpected extra node at the end: " + child->first);
    }
}
