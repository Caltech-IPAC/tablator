#include "../../Table.hxx"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../../Column.hxx"
#include "../../Common.hxx"
#include "../../Data_Type_Adjuster.hxx"
#include "../../Resource_Element.hxx"
#include "../../Utils/Vector_Utils.hxx"

namespace tablator {


void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &label, const Property &property,
                          bool allow_dups);

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &col_label, const Column &column,
                          const Data_Type &active_datatype, bool allow_dups);

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &col_label, const Column &column,
                          bool allow_dups);

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Group_Element &group, bool allow_dups);

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Resource_Element &resource_element,
                          const std::vector<Data_Type> &datatypes_for_writing,
                          bool allow_dups);

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Resource_Element &resource_element,
                          const std::vector<Data_Type> &datatypes_for_writing,
                          const std::vector<std::string> &comments,
                          const Labeled_Properties &table_labeled_properties,
                          bool allow_dups);


}  // namespace tablator

// JTODO Called for JSON, JSON5, VOTABLE.  Pick a default format and
// call get_datatypes_for_writing() instead of using original
// datatypes?  Who outside this repo actually calls this function?
boost::property_tree::ptree tablator::Table::generate_property_tree() const {
    return generate_property_tree(get_original_datatypes());
}

/**********************************************************/

boost::property_tree::ptree tablator::Table::generate_property_tree(
        const std::vector<Data_Type> &datatypes_for_writing, bool allow_dups) const {
    boost::property_tree::ptree outermost_tree;

    auto &votable_tree = outermost_tree.add(VOTABLE, "");
    votable_tree.add(XMLATTR + ".version", "1.3");
    votable_tree.add(XMLATTR + ".xmlns:xsi",
                     "http://www.w3.org/2001/XMLSchema-instance");
    votable_tree.add(XMLATTR + ".xmlns", "http://www.ivoa.net/xml/VOTable/v1.3");
    votable_tree.add(XMLATTR + ".xmlns:stc", "http://www.ivoa.net/xml/STC/v1.30");
    votable_tree.add(
            XMLATTR + ".xsi:schemaLocation",
            "http://www.ivoa.net/xml/VOTable/v1.3 http://www.ivoa.net/xml/VOTable/v1.3 "
            "http://www.ivoa.net/xml/STC/v1.30 http://www.ivoa.net/xml/STC/v1.30");

    if (!get_description().empty()) {
        votable_tree.add(DESCRIPTION, get_description());
    }

    for (const auto &name_and_prop : get_labeled_properties()) {
        const auto &name = name_and_prop.first;
        const auto &prop = name_and_prop.second;
        if (name == VOTABLE) continue;
        if (boost::starts_with(name, VOTABLE + ".")) {
            votable_tree.add(name.substr(VOTABLE.size() + 1), prop.get_value());
        } else if (is_property_style_label(name)) {
            // e.g. if came from ipac_table keyword
            auto &element = votable_tree.add(name, prop.get_value());
            for (auto &attr : prop.get_attributes()) {
                element.add(XMLATTR_DOT + attr.first, attr.second);
            }
        }
        // "else" is handled in add_to_property_tree() for Resource.
    }

    for (const auto &param : get_params()) {
        tablator::add_to_property_tree(votable_tree, PARAM, param, allow_dups);
    }


    for (const auto &group : get_group_elements()) {
        add_to_property_tree(votable_tree, group, allow_dups);
    }

    if (get_resource_elements().empty()) {
        throw std::runtime_error("no resource_elements");
    }

    for (const auto &resource_element : get_resource_elements()) {
        if (resource_element.is_results_resource()) {
            add_to_property_tree(votable_tree, resource_element, datatypes_for_writing,
                                 get_comments(), get_labeled_properties(), allow_dups);
        } else {
            add_to_property_tree(votable_tree, resource_element, datatypes_for_writing,
                                 allow_dups);
        }
    }

    for (const auto &info : get_trailing_info_list()) {
        add_to_property_tree(votable_tree, INFO, info, allow_dups);
    }

    return outermost_tree;
}
