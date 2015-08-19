#include "../../Table.hxx"

void tablator::Table::read_node_and_attributes
(const std::string &node_name,
 const boost::property_tree::ptree &node)
{
  Property property("");
  for (auto &xmlattr: node)
    {
      if (xmlattr.first == "<xmlattr>")
        for (auto &attribute: xmlattr.second)
          {
            property.attributes.insert
              (std::make_pair (attribute.first,
                               attribute.second.get_value<std::string> ()));
          }
    }
  std::cout << "node: " << node_name << ": '" << property.value << "'\n";
  for (auto &a: property.attributes)
    std::cout << "\t" << a.first << ": " << a.second << "\n";
  properties.emplace_back(node_name, property);
}
