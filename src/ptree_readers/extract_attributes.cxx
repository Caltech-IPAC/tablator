#include "../ptree_readers.hxx"

#include "../Common.hxx"

tablator::ATTRIBUTES tablator::ptree_readers::extract_attributes(
        const boost::property_tree::ptree &node) {
    tablator::ATTRIBUTES attributes;
    for (auto &child : node) {
        if (child.first == XMLATTR) {
            for (auto &attr : child.second) {
                attributes.insert(
                        std::make_pair(attr.first, attr.second.get_value<std::string>()));
            }
        }
    }
    return attributes;
}
