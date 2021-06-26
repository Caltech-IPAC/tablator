#include "../ptree_readers.hxx"

namespace tablator {


class Column;
class Property;
class Group_Element;


namespace ptree_readers {

void load_field_singleton(std::vector<tablator::Field> &params,
                          const boost::property_tree::ptree &node);

void load_field_array(std::vector<tablator::Field> &params,
                      const boost::property_tree::ptree &array_tree);


void load_labeled_properties_singleton(
        std::vector<std::pair<std::string, tablator::Property>> &labeled_properties,
        const std::string &element_label, const boost::property_tree::ptree &node);

void load_labeled_properties_array(
        std::vector<std::pair<std::string, tablator::Property>> &labeled_properties,
        const std::string &element_label,
        const boost::property_tree::ptree &array_tree);


void load_attributes_singleton(std::vector<tablator::ATTRIBUTES> &attrs,
                               const boost::property_tree::ptree &node);

void load_attributes_array(std::vector<tablator::ATTRIBUTES> &attrs,
                           const boost::property_tree::ptree &array_tree);


void load_group_element_singleton(std::vector<tablator::Group_Element> &group_elements,
                                  const boost::property_tree::ptree &node);

void load_group_element_array(std::vector<tablator::Group_Element> &group_elements,
                              const boost::property_tree::ptree &array_tree);


boost::property_tree::ptree::const_iterator read_links_section(
        std::vector<std::pair<std::string, tablator::Property>> &labeled_properties,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end, const std::string &next_tag);

boost::property_tree::ptree::const_iterator read_trailing_info_section(
        std::vector<tablator::Property> &trailing_info_list,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end);


}  // namespace ptree_readers
}  // namespace tablator
