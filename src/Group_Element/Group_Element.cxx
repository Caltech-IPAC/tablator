#include "../Group_Element.hxx"

#include "../ptree_readers.hxx"

namespace tablator {
Group_Element::Builder &Group_Element::Builder::add_params(
        const std::string &params_xml) {
    if (params_xml.empty()) {
        return *this;
    }
    const auto params_tree =
            tablator::ptree_readers::read_string_as_property_tree(params_xml);
    boost::property_tree::ptree::const_iterator child = params_tree.begin();
    boost::property_tree::ptree::const_iterator end = params_tree.end();
    while (child != end) {
        if (child->first == PARAM) {
            options_.params_.emplace_back(
                    ptree_readers::read_field(child->second).get_field());
        }
        ++child;
    }
    return *this;
}
}  // namespace tablator
