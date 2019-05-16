#include "../../Table.hxx"

void tablator::Table::read_node_and_attributes(
        const std::string &node_name, const boost::property_tree::ptree &node) {
    Property property("");
    for (auto &xmlattr : node) {
        if (xmlattr.first == "<xmlattr>")
            for (auto &attribute : xmlattr.second) {
                property.attributes.insert(std::make_pair(
                        attribute.first, attribute.second.get_value<std::string>()));
            }
    }
    properties.emplace_back(node_name, property);
}
