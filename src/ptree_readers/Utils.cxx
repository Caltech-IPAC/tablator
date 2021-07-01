#include "Utils.hxx"

#include "../Column.hxx"
#include "../Group_Element.hxx"
#include "../Property.hxx"

namespace {


void load_link_singleton(
        std::vector<std::pair<std::string, tablator::Property>> &labeled_properties,
        const boost::property_tree::ptree &node) {
    labeled_properties.emplace_back(std::make_pair(
            tablator::LINK,
            tablator::Property(tablator::ptree_readers::extract_attributes(node))));
}


void load_trailing_info_singleton(std::vector<tablator::Property> &trailing_info,
                                  const boost::property_tree::ptree &node) {
    trailing_info.emplace_back(tablator::ptree_readers::read_property(node));
}


}  // namespace

namespace tablator {
namespace ptree_readers {


void load_attributes_singleton(std::vector<tablator::ATTRIBUTES> &attrs,
                               const boost::property_tree::ptree &node) {
    attrs.emplace_back(tablator::ptree_readers::extract_attributes(node));
}


boost::property_tree::ptree::const_iterator read_links_section(
        std::vector<std::pair<std::string, tablator::Property>> &labeled_properties,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end, const std::string &next_tag) {
    boost::property_tree::ptree::const_iterator &iter = start;

    while (iter != end) {
        if (iter->first == tablator::LINK) {
            load_link_singleton(labeled_properties, iter->second);
        } else if (iter->first == next_tag) {
            break;
        } else {
            throw std::runtime_error(std::string("Expected LINK or TABLE tag, "
                                                 "but found ") +
                                     iter->first);
        }
        ++iter;
    }
    return iter;
}


void load_field_singleton(std::vector<tablator::Field> &params,
                          const boost::property_tree::ptree &node) {
    params.emplace_back(tablator::ptree_readers::read_field(node).get_field());
}

void load_labeled_properties_singleton(
        std::vector<std::pair<std::string, tablator::Property>> &labeled_properties,
        const std::string &element_label, const boost::property_tree::ptree &node) {
    labeled_properties.emplace_back(std::make_pair(
            element_label, tablator::ptree_readers::read_property(node)));
}


boost::property_tree::ptree::const_iterator read_trailing_info_section(
        std::vector<tablator::Property> &trailing_info_list,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end) {
    boost::property_tree::ptree::const_iterator &iter = start;

    while (iter != end) {
        if (iter->first == tablator::INFO) {
            load_trailing_info_singleton(trailing_info_list, iter->second);
        } else {
            throw std::runtime_error(std::string("Expected INFO tag, if "
                                                 "anything, but found ") +
                                     iter->first);
        }
        ++iter;
    }
    return iter;
}


void load_group_element_singleton(std::vector<tablator::Group_Element> &group_elements,
                                  const boost::property_tree::ptree &node) {
    group_elements.emplace_back(read_group_element(node));
}


}  // namespace ptree_readers
}  // namespace tablator
