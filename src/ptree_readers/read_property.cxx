#include "../ptree_readers.hxx"

#include <boost/lexical_cast.hpp>

#include "../Property.hxx"

tablator::Property tablator::ptree_readers::read_property(
        const boost::property_tree::ptree &prop_tree) {
    const auto attributes = extract_attributes(prop_tree);

    // Tables in VOTABLE format store <value> elements in property_trees
    // as node values; tables in JSON format store them as labeled children.
    boost::optional<const boost::property_tree::ptree &> value_tree_opt =
            prop_tree.get_child_optional(tablator::VALUE);
    const auto value = value_tree_opt ? value_tree_opt.get().get_value<std::string>()
                                      : prop_tree.get_value<std::string>();

    return Property(value, attributes);
}
