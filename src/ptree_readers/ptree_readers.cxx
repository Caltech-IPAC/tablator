#include "../ptree_readers.hxx"

#include <boost/property_tree/xml_parser.hpp>

boost::property_tree::ptree tablator::ptree_readers::read_xml_string_as_property_tree(
        const std::string &ptree_xml) {
    std::istringstream iss(ptree_xml);
    using namespace boost::property_tree::xml_parser;
    boost::property_tree::ptree ptree;
    boost::property_tree::read_xml(iss, ptree, trim_whitespace | no_comments);
    return ptree;
}

void tablator::ptree_readers::add_params_from_xml_string(
        std::vector<Field> &params, const std::string &params_xml) {
    if (params_xml.empty()) {
        return;
    }
    const auto params_tree = read_xml_string_as_property_tree(params_xml);
    boost::property_tree::ptree::const_iterator child = params_tree.begin();
    boost::property_tree::ptree::const_iterator end = params_tree.end();
    while (child != end) {
        if (child->first == PARAM) {
            params.emplace_back(read_field(child->second));
        }
        ++child;
    }
}
