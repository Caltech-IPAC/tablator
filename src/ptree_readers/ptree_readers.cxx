#include "../ptree_readers.hxx"

#include <boost/property_tree/xml_parser.hpp>

boost::property_tree::ptree tablator::ptree_readers::read_string_as_property_tree(
        const std::string &ptree_xml) {
    std::istringstream iss(ptree_xml);
    using namespace boost::property_tree::xml_parser;
    boost::property_tree::ptree ptree;
    boost::property_tree::read_xml(iss, ptree, trim_whitespace | no_comments);
    return ptree;
}
