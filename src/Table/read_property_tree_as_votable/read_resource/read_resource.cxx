#include "../../../Table.hxx"

#include "../skip_xml_comments.hxx"
#include "VOTable_Field.hxx"

void tablator::Table::read_resource(const boost::property_tree::ptree &resource) {
    auto child = resource.begin();
    auto end = resource.end();

    add_labeled_property(RESOURCE, Property(extract_attributes(resource)));
    child = skip_xml_comments(child, end);
    while (child != end && child->first == XMLATTR) {
        ++child;
        child = skip_xml_comments(child, end);
    }
    if (child != end && child->first == DESCRIPTION) {
        add_labeled_property("RESOURCE.DESCRIPTION",
                             child->second.get_value<std::string>());
        ++child;
    }
    child = skip_xml_comments(child, end);
    while (child != end && child->first == INFO) {
        add_labeled_property("RESOURCE.INFO", read_property(child->second));
        ++child;
        child = skip_xml_comments(child, end);
    }
    while (child != end) {
        if (child->first == COOSYS) {
            add_labeled_property("RESOURCE.COOSYS", read_property(child->second));
        } else if (child->first == PARAM) {
            add_resource_element_param(read_field(child->second));
        } else if (child->first == GROUP) {
            // FIXME: Implement groups
        } else {
            break;
        }
        ++child;
        child = skip_xml_comments(child, end);
    }
    /// We only allow one TABLE per RESOURCE
    auto &columns = get_columns();
    for (; child != end; ++child) {
        if (child->first == LINK) {
            add_labeled_property("RESOURCE.LINK", read_property(child->second));
        } else if (child->first == "TABLE") {
            if (!columns.empty())
                throw std::runtime_error(
                        "Multiple TABLE elements are not implemented.");
            read_table(child->second);
        } else if (child->first == INFO) {
            add_labeled_property("RESOURCE.INFO", read_property(child->second));
        }
        /// skip <xmlattr> and <xmlcomment>
    }
}
