#include "../../Table.hxx"

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../../Data_Type_Adjuster.hxx"


namespace tablator {
void add_to_property_tree(const Column &column, const std::string &tree_name,
                          boost::property_tree::ptree &tree,
                          const Data_Type &active_datatype);

void add_to_property_tree(const Column &column, const std::string &tree_name,
                          boost::property_tree::ptree &tree) {
    add_to_property_tree(column, tree_name, tree, column.get_type());
}
}  // namespace tablator


// JTODO Called for JSON, JSON5, VOTABLE.  Pick a default format and
// call get_datatypes_for_writing() instead of using original
// datatypes?  Who outside this repo actually calls this function?
boost::property_tree::ptree tablator::Table::generate_property_tree(
    const std::string &tabledata_string) const {
    return generate_property_tree(tabledata_string, get_original_datatypes());
}

/**********************************************************/

boost::property_tree::ptree tablator::Table::generate_property_tree(
    const std::string &tabledata_string,
    const std::vector<Data_Type> &datatypes_for_writing) const {
    boost::property_tree::ptree tree;
    auto &votable = tree.add(VOTABLE, "");
    votable.add("<xmlattr>.version", "1.3");
    votable.add("<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    votable.add("<xmlattr>.xmlns", "http://www.ivoa.net/xml/VOTable/v1.3");
    votable.add("<xmlattr>.xmlns:stc", "http://www.ivoa.net/xml/STC/v1.30");
    votable.add(
            "<xmlattr>.xsi:schemaLocation",
            "http://www.ivoa.net/xml/VOTable/v1.3 http://www.ivoa.net/xml/VOTable/v1.3 "
            "http://www.ivoa.net/xml/STC/v1.30 http://www.ivoa.net/xml/STC/v1.30");

    bool overflow = false;

    auto &resource = votable.add(RESOURCE, "");
    for (auto &p : get_labeled_properties()) {
        if (p.first == VOTABLE) continue;
        if (boost::starts_with(p.first, VOTABLE + ".")) {
            votable.add(p.first.substr(VOTABLE.size() + 1), p.second.get_value());
        } else if (is_property_style_label(p.first)) {
            auto &element = votable.add(p.first, p.second.get_value());
            for (auto &a : p.second.get_attributes())
                element.add(XMLATTR_DOT + a.first, a.second);
        } else if (p.first == RESOURCE) {
            for (auto &a : p.second.get_attributes())
                resource.add(XMLATTR_DOT + a.first, a.second);
        } else if (boost::starts_with(p.first, RESOURCE + "." + TABLE)) {
            /// Skip TABLE for now.
        } else if (boost::starts_with(p.first, RESOURCE + ".")) {
            auto &element =
                    resource.add(p.first.substr(RESOURCE.size() + 1), p.second.get_value());
            for (auto &a : p.second.get_attributes())
                element.add(XMLATTR_DOT + a.first, a.second);
        } else if (p.first == "OVERFLOW") {
            overflow = true;
        } else {
            auto &info = resource.add(INFO, "");
            info.add("<xmlattr>.name", p.first);
            info.add("<xmlattr>.value", p.second.get_value());
            for (auto &a : p.second.get_attributes()) {
                auto &info_attribute = resource.add(INFO, "");
                info_attribute.add("<xmlattr>.name", p.first + "." + a.first);
                info_attribute.add("<xmlattr>.value", a.second);
            }
        }
    }
    for (auto &param : get_resource_element_params()) {
        add_to_property_tree(param, PARAM, resource);
    }

    boost::property_tree::ptree &table_tree = resource.add(TABLE, "");

    // VOTable only allows a single DESCRIPTION element, so we combine
    // RESOURCE.TABLE.DESCRIPTION and comments in a single string.
    std::string description;
    for (auto &p : get_labeled_properties()) {
        if (boost::starts_with(p.first, RESOURCE + "." + TABLE)) {
            for (auto &a : p.second.get_attributes()) {
                table_tree.add(XMLATTR_DOT + a.first, a.second);
            }
            if (boost::starts_with(p.first,
                                   RESOURCE + "." + TABLE + "." + DESCRIPTION)) {
                if (!description.empty()) {
                    description.append("\n");
                }
                description.append(p.second.get_value());
            }
        }
    }
    const auto &comments = get_comments();
    if (!comments.empty()) {
        if (!description.empty()) {
            description.append("\n");
        }
        description.append(boost::join(comments, "\n"));
    }

    if (!description.empty()) {
        table_tree.add(DESCRIPTION, description);
    }

    for (auto &param : get_table_element_params()) {
        add_to_property_tree(param, PARAM, table_tree);
    }
    /// Skip null_bitfield_flag
    const auto &columns = get_columns();
    for (size_t i = 1; i < columns.size(); ++i) {
        add_to_property_tree(columns[i], FIELD, table_tree, datatypes_for_writing[i]);
    }

    table_tree.add("DATA.TABLEDATA", tabledata_string);
    if (overflow) {
        auto &info = tree.add("VOTABLE.RESOURCE.INFO", "");
        info.add("<xmlattr>.name", QUERY_STATUS);
        info.add("<xmlattr>.value", "OVERFLOW");
    }

    return tree;
}
