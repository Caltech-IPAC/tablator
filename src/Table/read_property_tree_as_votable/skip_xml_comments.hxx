#include <boost/property_tree/xml_parser.hpp>

namespace tablator {
inline boost::property_tree::ptree::const_iterator skip_xml_comments(
        const boost::property_tree::ptree::const_iterator &old_child,
        const boost::property_tree::ptree::const_iterator &end) {
    auto child(old_child);
    while (child != end && child->first == "<xmlcomment>") {
        ++child;
    }
    return child;
}
}  // namespace tablator
