#pragma once

#include <fstream>
#include <string>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include "Column.hxx"
#include "Common.hxx"
#include "Field_Framework.hxx"

namespace tablator {

class Property;
class Data_Element;
class Group_Element;
class Resource_Element;
class Table_Element;
class Table;


//===============================================================================
// property_trees read by the top-level
// read_property_tree_as_votable() function are assumed to be in
// VOTable format as defined here:
// https://www.ivoa.net/documents/VOTable/20191021/REC-VOTable-1.4-20191021.html.
// ===============================================================================

namespace ptree_readers {

ATTRIBUTES extract_attributes(const boost::property_tree::ptree &node);

// The motivation for the "_element" business is distinguishing
// Table from Table_Element.

void read_property_tree_as_votable(Table &table,
                                   const boost::property_tree::ptree &tree);

Resource_Element read_resource_element(const boost::property_tree::ptree &resource_tree,
                                       bool &is_results_resource);
Group_Element read_group_element(const boost::property_tree::ptree &node);
Table_Element read_table_element(const boost::property_tree::ptree &table);
Field read_field(const boost::property_tree::ptree &field);
Property read_property(const boost::property_tree::ptree &prop);

Data_Element read_data_element(const boost::property_tree::ptree &data,
                               const std::vector<Field> &fields);
Data_Element read_tabledata(const boost::property_tree::ptree &tabledata,
                            const std::vector<Field> &fields);
Data_Element read_binary2(const boost::property_tree::ptree &binary2,
                          const std::vector<Field> &fields);

void append_data_from_stream(std::vector<uint8_t> &data,
                             const Field_Framework &field_framework,
                             const std::vector<uint8_t> &stream,
                             const std::vector<Field> &fields, size_t num_rows);

boost::property_tree::ptree read_xml_string_as_property_tree(
        const std::string &ptree_xml);
void add_params_from_xml_string(std::vector<Field> &params,
                                const std::string &params_xml);


}  // namespace ptree_readers
}  // namespace tablator
