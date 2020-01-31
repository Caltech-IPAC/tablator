#include "../ptree_readers.hxx"

#include <boost/lexical_cast.hpp>

#include "../Common.hxx"

tablator::Property tablator::ptree_readers::read_property(
        const boost::property_tree::ptree &prop_tree) {
    auto child = prop_tree.begin();
    auto end = prop_tree.end();


    std::string value;
    tablator::ATTRIBUTES attributes;

    while (child != end) {
        if (child->first == XMLATTR) {
            for (auto &att : child->second) {
                attributes.insert(
                        std::make_pair(att.first, att.second.get_value<std::string>()));
            }
        } else {
            // INFO must not contain subelement
            value.assign(child->second.get_value<std::string>());
        }
        ++child;
    }

    return Property(value, attributes);
}
