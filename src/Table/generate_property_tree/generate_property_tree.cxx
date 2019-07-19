#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../../Data_Type_Adjuster.hxx"
#include "../../Table.hxx"

namespace tablator {
void add_to_property_tree(const Column &column, const std::string &tree_name,
                          boost::property_tree::ptree &tree,
                          const Data_Type &active_datatype);

void add_to_property_tree(const Column &column, const std::string &tree_name,
                          boost::property_tree::ptree &tree) {
    add_to_property_tree(column, tree_name, tree, column.type);
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
    std::string votable_literal("VOTABLE");
    auto &votable = tree.add(votable_literal, "");
    votable.add("<xmlattr>.version", "1.3");
    votable.add("<xmlattr>.xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    votable.add("<xmlattr>.xmlns", "http://www.ivoa.net/xml/VOTable/v1.3");
    votable.add("<xmlattr>.xmlns:stc", "http://www.ivoa.net/xml/STC/v1.30");
    votable.add(
            "<xmlattr>.xsi:schemaLocation",
            "http://www.ivoa.net/xml/VOTable/v1.3 http://www.ivoa.net/xml/VOTable/v1.3 "
            "http://www.ivoa.net/xml/STC/v1.30 http://www.ivoa.net/xml/STC/v1.30");

    bool overflow = false;

    const std::string resource_literal("RESOURCE");
    const std::string table_literal("TABLE");
    const std::string description_literal("DESCRIPTION");

    auto &resource = votable.add(resource_literal, "");
    for (auto &p : properties) {
        if (p.first == votable_literal) continue;
        if (boost::starts_with(p.first, votable_literal + ".")) {
            votable.add(p.first.substr(votable_literal.size() + 1), p.second.value);
        } else if (p.first == "COOSYS" || p.first == "GROUP" || p.first == "PARAM" ||
                   p.first == "INFO") {
            auto &element = votable.add(p.first, p.second.value);
            for (auto &a : p.second.attributes)
                element.add("<xmlattr>." + a.first, a.second);
        } else if (p.first == resource_literal) {
            for (auto &a : p.second.attributes)
                resource.add("<xmlattr>." + a.first, a.second);
        } else if (boost::starts_with(p.first,
                                      resource_literal + "." + table_literal)) {
            /// Skip TABLE for now.
        } else if (boost::starts_with(p.first, resource_literal + ".")) {
            auto &element = resource.add(p.first.substr(resource_literal.size() + 1),
                                         p.second.value);
            for (auto &a : p.second.attributes)
                element.add("<xmlattr>." + a.first, a.second);
        } else if (p.first == "OVERFLOW") {
            overflow = true;
        } else {
            auto &info = resource.add("INFO", "");
            info.add("<xmlattr>.name", p.first);
            info.add("<xmlattr>.value", p.second.value);
            for (auto &a : p.second.attributes) {
                auto &info_attribute = resource.add("INFO", "");
                info_attribute.add("<xmlattr>.name", p.first + "." + a.first);
                info_attribute.add("<xmlattr>.value", a.second);
            }
        }
    }
    for (auto &param : resource_params) {
        add_to_property_tree(param, "PARAM", resource);
    }

    boost::property_tree::ptree &table_tree = resource.add(table_literal, "");

    // VOTable only allows a single DESCRIPTION element, so we combine
    // RESOURCE.TABLE.DESCRIPTION and comments in a single string.
    std::string description;
    for (auto &p : properties) {
        if (boost::starts_with(p.first, resource_literal + "." + table_literal)) {
            for (auto &a : p.second.attributes) {
                table_tree.add("<xmlattr>." + a.first, a.second);
            }
            if (boost::starts_with(p.first, resource_literal + "." + table_literal +
                                                    "." + description_literal)) {
                if (!description.empty()) {
                    description.append("\n");
                }
                description.append(p.second.value);
            }
        }
    }
    if (!comments.empty()) {
        if (!description.empty()) {
            description.append("\n");
        }
        description.append(boost::join(comments, "\n"));
    }

    if (!description.empty()) {
        table_tree.add("DESCRIPTION", description);
    }

    for (auto &param : table_params) {
        add_to_property_tree(param, "PARAM", table_tree);
    }
    /// Skip null_bitfield_flag
    for (size_t i = 1; i < columns.size(); ++i) {
        add_to_property_tree(columns[i], "FIELD", table_tree, datatypes_for_writing[i]);
    }
    table_tree.add("DATA.TABLEDATA", tabledata_string);
    if (overflow) {
        auto &info = tree.add("VOTABLE.RESOURCE.INFO", "");
        info.add("<xmlattr>.name", "QUERY_STATUS");
        info.add("<xmlattr>.value", "OVERFLOW");
    }

    return tree;
}
