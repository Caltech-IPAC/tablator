#include "../Table.hxx"

#include "../ptree_readers.hxx"
#include "../ptree_readers/read_resource/VOTable_Field.hxx"  // JTODO because of read_field()


// This only parses VOTable v1.3.  // JTODO 1.4 now?

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

    child = ptree_readers::skip_xml_comments(child, end);
    if (child != end && child->first == DESCRIPTION) {
        table.set_description(child->second.get_value<std::string>());
        ++child;
    }
    child = ptree_readers::skip_xml_comments(child, end);
    if (child != end && child->first == DEFINITIONS) {
        /// Deliberately ignore DEFINITIONS.  They are duplicated by the
        /// information in RESOURCE and deprecated since version 1.1.
        ++child;
    }

    child = ptree_readers::skip_xml_comments(child, end);
    while (child != end && child->first != RESOURCE) {
        if ((child->first == COOSYS) || (child->first == INFO)) {
            table.add_labeled_property(child->first,
                                       ptree_readers::read_property(child->second));
        } else if (child->first == PARAM) {
            table.add_param(ptree_readers::read_field(child->second));
        } else if (child->first == GROUP) {
            table.add_group_element(ptree_readers::read_group(child->second));
        } else {
            throw std::runtime_error(
                    "In VOTABLE, expected COOSYS, GROUP, "
                    "PARAM, INFO, RESOURCE, or a comment, but got: " +
                    child->first);
        }
        ++child;
        child = ptree_readers::skip_xml_comments(child, end);
    }

    if (child == end) {
        throw std::runtime_error("Missing RESOURCE in VOTABLE");
    } else {
        table.add_resource_element(
                ptree_readers::read_resource(child->second, true /* is_first */));
    }
    ++child;
    child = ptree_readers::skip_xml_comments(child, end);

    // read secondary resources
    while (child != end && child->first == RESOURCE) {
        table.add_resource_element(
                ptree_readers::read_resource(child->second, false /* is_first */));
        ++child;
        child = ptree_readers::skip_xml_comments(child, end);
    }

    while (child != end) {
        if (child->first == INFO) {
            auto curr_attributes = ptree_readers::extract_attributes(child->second);
            table.add_trailing_info(Property(curr_attributes));
        } else {
            throw std::runtime_error(
                    "In VOTABLE, expected INFO tag, if anything, "
                    "but got: " +
                    child->first);
        }
        ++child;
        child = ptree_readers::skip_xml_comments(child, end);
    }
}
