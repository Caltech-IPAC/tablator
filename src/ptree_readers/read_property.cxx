#include "../ptree_readers.hxx"

#include <boost/lexical_cast.hpp>

#include "../Property.hxx"

tablator::Property tablator::ptree_readers::read_property(
        const boost::property_tree::ptree &prop_tree) {
    const auto attributes = extract_attributes(prop_tree);
    const auto value = prop_tree.get_value<std::string>();

    return Property(value, attributes);
}
