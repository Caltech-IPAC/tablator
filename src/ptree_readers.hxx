#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include "Common.hxx"
#include "Data_Element.hxx"
#include "Group_Element.hxx"
#include "Property.hxx"
#include "Resource_Element.hxx"
#include "Table_Element.hxx"
#include "Utils/Table_Utils.hxx"


namespace tablator {
class Table;

namespace ptree_readers {

ATTRIBUTES extract_attributes(const boost::property_tree::ptree &node);

class Field_And_Flag {
public:
    Field_And_Flag(const tablator::Field &field, bool flag)
            : field_(field), flag_(flag) {}
    const tablator::Field &get_field() const { return field_; }
    bool get_dynamic_array_flag() const { return flag_; }

private:
    tablator::Field field_;
    bool flag_;
};

// The motivation for the "_element" business is distinguishing
// Table from Table_Element. JTODO rename to read_XXX_element
// after dust settles?

void read_property_tree_as_votable(Table &table,
                                   const boost::property_tree::ptree &tree);

Resource_Element read_resource(const boost::property_tree::ptree &resource_tree,
                               bool is_first);
Group_Element read_group(const boost::property_tree::ptree &node);
Table_Element read_table(const boost::property_tree::ptree &table);
Field_And_Flag read_field(const boost::property_tree::ptree &field);
Property read_property(const boost::property_tree::ptree &prop);

Data_Element read_data(const boost::property_tree::ptree &data,
                       const std::vector<Field_And_Flag> &field_flag_pairs);
Data_Element read_tabledata(const boost::property_tree::ptree &tabledata,
                            const std::vector<Field_And_Flag> &field_flag_pairs);
Data_Element read_binary2(const boost::property_tree::ptree &binary2,
                          const std::vector<Field_And_Flag> &field_flag_pairs);

void append_data_from_stream(std::vector<uint8_t> &data,
                             const std::vector<Column> &columns,
                             const std::vector<size_t> &offsets,
                             const std::vector<uint8_t> &stream,
                             const std::vector<Field_And_Flag> &field_flag_pairs,
                             size_t num_rows);


boost::property_tree::ptree::const_iterator skip_xml_comments(
        const boost::property_tree::ptree::const_iterator &old_child,
        const boost::property_tree::ptree::const_iterator &end);

boost::property_tree::ptree read_string_as_property_tree(const std::string &ptree_xml);

}  // namespace ptree_readers
}  // namespace tablator
