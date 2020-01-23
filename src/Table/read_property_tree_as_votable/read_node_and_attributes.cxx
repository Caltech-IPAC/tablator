#include "../../Table.hxx"

void tablator::Table::read_node_and_attributes( // JTODO rename
        const std::string &node_name, const boost::property_tree::ptree &node) {
    Property property("");
    for (auto &xmlattr : node) {
        if (xmlattr.first == "<xmlattr>")
            for (auto &attribute : xmlattr.second) {
                property.add_attribute(
                        attribute.first, attribute.second.get_value<std::string>());
            }
    }
    add_labeled_property(std::make_pair(node_name, property));
}
