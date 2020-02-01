#include "../ptree_readers.hxx"

#include <boost/property_tree/xml_parser.hpp>


boost::property_tree::ptree::const_iterator tablator::ptree_readers::skip_xml_comments(
        const boost::property_tree::ptree::const_iterator &old_child,
        const boost::property_tree::ptree::const_iterator &end) {
    auto child(old_child);
    while (child != end && child->first == XMLCOMMENT) {
        ++child;
    }
    return child;
}
