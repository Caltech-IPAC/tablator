#include "../ptree_readers.hxx"

#include "../Common.hxx"

tablator::ATTRIBUTES tablator::ptree_readers::extract_attributes(
        const boost::property_tree::ptree &node) {
    tablator::ATTRIBUTES attributes;
    for (auto &child : node) {
        if (child.first == XMLATTR) {
            for (auto &att : child.second) {
                attributes.insert(
                        std::make_pair(att.first, att.second.get_value<std::string>()));
            }
        }
    }
    return attributes;
}
