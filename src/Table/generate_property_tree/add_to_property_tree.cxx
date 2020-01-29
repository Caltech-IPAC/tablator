#include "../../Table.hxx"

#include <boost/property_tree/ptree.hpp>

#include "../../Column.hxx"
#include "../../Common.hxx"
#include "../../Group_Element.hxx"
#include "../../Property.hxx"
#include "../../Resource_Element.hxx"
#include "../../Table_Element.hxx"
#include "../../to_xml_string.hxx"

namespace {
void Min_Max_to_xml(boost::property_tree::ptree &tree, const std::string &min_max,
                    const tablator::Min_Max &m) {
    if (!m.empty()) {
        boost::property_tree::ptree &min_max_tree = tree.add(min_max, "");
        min_max_tree.add(tablator::XMLATTR + ".value", m.value);
        min_max_tree.add(tablator::XMLATTR + ".inclusive", m.inclusive ? "yes" : "no");
    }
}

void Option_to_xml(boost::property_tree::ptree &tree, const tablator::Option &option) {
    if (!option.empty()) {
        boost::property_tree::ptree &option_tree = tree.add("OPTION", "");
        if (!option.name.empty()) {
            option_tree.add(tablator::XMLATTR + ".name", option.name);
        }
        if (!option.value.empty()) {
            option_tree.add(tablator::XMLATTR + ".value", option.value);
        }
        for (auto &o : option.options) Option_to_xml(option_tree, o);
    }
}
}  // namespace

namespace tablator {

//==============================================================
//*** ATTRIBUTES
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const tablator::ATTRIBUTES &attributes) {
    for (const auto &att_pair : attributes) {
        parent_tree.add(tablator::XMLATTR + "." + att_pair.first, att_pair.second);
    }
}

//==============================================================
//*** Property
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &label, const Property &property) {
    const auto &value = property.get_value();
    const auto &attributes = property.get_attributes();
    if (is_property_style_label(label)) {
        auto &label_tree = parent_tree.add(label, value);
        add_to_property_tree(label_tree, attributes);
    } else if (label.empty()) {
        // JTODO ignore value?  Error if there is a value?
        add_to_property_tree(parent_tree, attributes);
    } else if (boost::starts_with(label, XMLATTR)) {
        parent_tree.add(label, value);
    } else {
        // Backward compatibility.
        auto &label_tree = parent_tree.add(INFO, "");
        label_tree.add(XMLATTR + ".name", label);
        label_tree.add(XMLATTR + ".value", value);
        add_to_property_tree(label_tree, attributes);
    }
}

//==============================================================
//*** Labeled_Properties
void add_to_property_tree(
        boost::property_tree::ptree &parent_tree,
        const std::vector<std::pair<std::string, Property>> &labeled_properties) {
    for (const auto &labeled_list : labeled_properties) {
        const auto &label = labeled_list.first;
        add_to_property_tree(parent_tree, label, labeled_list.second);
    }
}


//==============================================================
//*** Column
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &col_label, const Column &column,
                          const Data_Type &active_datatype) {
    boost::property_tree::ptree &field_tree = parent_tree.add(col_label, "");
    field_tree.add(XMLATTR + ".name", column.get_name());
    std::string datatype = to_xml_string(active_datatype);
    field_tree.add(XMLATTR + ".datatype", datatype);

    bool added_arraysize =
            (active_datatype == Data_Type::CHAR || column.get_array_size() != 1);
    if (added_arraysize) {
        field_tree.add("<xmlattr>.arraysize", "*");
    }

    const auto &field_properties = column.get_field_properties();
    for (auto &a : field_properties.get_attributes()) {
        /// Empty attributes cause field_tree.add to crash :(, so make sure
        /// that does not happen.

        // FIXME: This error is thrown a bit too late to be useful.

        if (a.first.empty())
            throw std::runtime_error("Empty attribute in field " + column.get_name() +
                                     " which has type " + to_string(column.get_type()));
        if (added_arraysize && boost::equals(a.first, "arraysize")) {
            continue;
        }
        field_tree.add(XMLATTR + "." + a.first, a.second);
    }

    const auto &desc = field_properties.get_description();
    if (!desc.empty()) {
        field_tree.add(DESCRIPTION, desc);
    }

    auto &v(field_properties.get_values());
    if (!v.empty_except_null()) {
        boost::property_tree::ptree &values = field_tree.add("VALUES", "");
        if (!v.ID.empty()) values.add(XMLATTR + ".ID", v.ID);
        if (!v.type.empty()) values.add(XMLATTR + ".type", v.type);
        if (!v.ref.empty()) values.add(XMLATTR + ".ref", v.ref);

        Min_Max_to_xml(values, "MIN", v.min);
        Min_Max_to_xml(values, "MAX", v.max);

        for (auto &o : v.options) Option_to_xml(values, o);
    }

    if (!field_properties.get_links().empty()) {
        boost::property_tree::ptree &link_tree = field_tree.add("LINK", "");
        for (auto &link : field_properties.get_links()) {
            link_tree.add(XMLATTR + "." + link.first, link.second);
        }
    }
}

//==============================================================

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &col_label, const Column &column) {
    add_to_property_tree(parent_tree, col_label, column, column.get_type());
}


//==============================================================
//*** Group_Element
void add_to_property_tree(boost::property_tree::ptree &tree,
                          const Group_Element &group_element) {
    boost::property_tree::ptree &group_tree = tree.add(GROUP, "");
    for (const auto &pair : group_element.get_attributes()) {
        group_tree.add(XMLATTR + "." + pair.first, pair.second);
    }
    if (!group_element.get_description().empty()) {
        group_tree.add(DESCRIPTION, group_element.get_description());
    }

    for (const auto &param : group_element.get_params()) {
        add_to_property_tree(group_tree, PARAM, param);
    }

    for (const auto &att_map : group_element.get_field_refs()) {
        boost::property_tree::ptree &field_ref_tree = group_tree.add(FIELDREF, "");
        for (const auto &att : att_map) {
            field_ref_tree.add(XMLATTR + "." + att.first, att.second);
        }
    }
}


//==============================================================
//*** Table_Element
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Table_Element &table_element,
                          const std::vector<Data_Type> &datatypes_for_writing,
                          const std::vector<std::string> &comments) {
    boost::property_tree::ptree &table_tree = parent_tree.add(TABLE, "");

    for (const auto &pair : table_element.get_attributes()) {
        table_tree.add(XMLATTR + "." + pair.first, pair.second);
    }
    // VOTable allows Table_Element only a single DESCRIPTION element, so we combine
    // RESOURCE.TABLE.DESCRIPTION and comments in a single string.
    // JTODO comments could go in INFO
    std::string table_description = table_element.get_description();

    if (!comments.empty()) {
        if (!table_description.empty()) {
            table_description.append("\n");
        }
        table_description.append(boost::join(comments, "\n"));
    }

    if (!table_description.empty()) {
        table_tree.add(DESCRIPTION, table_description);
    }


    for (const auto &group : table_element.get_group_elements()) {
        add_to_property_tree(table_tree, group);
    }

    for (auto &param : table_element.get_params()) {
        add_to_property_tree(table_tree, PARAM, param);
    }

    const auto columns = table_element.get_columns();

    /// Skip null_bitfield_flag
    for (size_t i = 1; i < columns.size(); ++i) {
        add_to_property_tree(table_tree, FIELD, columns[i], datatypes_for_writing[i]);
    }


    // We don't add tabledata directly because it is not in ptree format.
    table_tree.add(DATA_TABLEDATA, TABLEDATA_PLACEHOLDER);
    for (const auto &info : table_element.get_trailing_info_list()) {
        add_to_property_tree(table_tree, INFO, info);
    }
}

//==============================================================
//*** Resource_Element

void add_to_property_tree(
        boost::property_tree::ptree &parent_tree,
        const Resource_Element &resource_element,
        const std::vector<Data_Type> &datatypes_for_writing, size_t resource_id,
        const std::vector<std::string> &comments,
        const std::vector<std::pair<std::string, Property>> &table_labeled_properties) {
    boost::property_tree::ptree &resource_tree = parent_tree.add(RESOURCE, "");
    for (const auto &pair : resource_element.get_attributes()) {
        resource_tree.add(XMLATTR + "." + pair.first, pair.second);
    }
    if (!resource_element.get_description().empty()) {
        resource_tree.add(DESCRIPTION, resource_element.get_description());
    }

    // JTODO backward compatibility-ish.  Ick.
    for (const auto &name_and_prop : table_labeled_properties) {
        const auto &name = name_and_prop.first;
        const auto &prop = name_and_prop.second;
        if (boost::starts_with(name, VOTABLE_RESOURCE_DOT)) {
            add_to_property_tree(resource_tree,
                                 name.substr(VOTABLE_RESOURCE_DOT.size() + 1), prop);
        }
    }

    if (!resource_element.get_labeled_properties().empty()) {
        add_to_property_tree(resource_tree, resource_element.get_labeled_properties());
    }

    for (const auto &group : resource_element.get_group_elements()) {
        add_to_property_tree(resource_tree, group);
    }

    for (const auto &ptree : resource_element.get_params_ptree()) {
        resource_tree.add_child(PARAM, ptree.second);
    }
    for (const auto &param : resource_element.get_params()) {
        add_to_property_tree(resource_tree, PARAM, param);
    }

    if (resource_id == TABLE_RESOURCE_IDX &&
        !resource_element.get_table_elements()
                 .empty()) {  // JTODO shouldn't have to check
        // add only one table_element
        const auto &table_element_0 = resource_element.get_table_elements().at(0);
        add_to_property_tree(resource_tree, table_element_0, datatypes_for_writing,
                             comments);
    }

    for (const auto &info : resource_element.get_trailing_info_list()) {
        add_to_property_tree(resource_tree, INFO, info);
    }
}


//==============================================================
//*** Resource_Element

static const std::vector<std::string> DEFAULT_COMMENTS = std::vector<std::string>();
static const std::vector<std::pair<std::string, Property>>
        DEFAULT_TABLE_LABELED_PROPERTIES =
                std::vector<std::pair<std::string, Property>>();

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Resource_Element &resource_element,
                          const std::vector<Data_Type> &datatypes_for_writing,
                          size_t resource_id) {
    add_to_property_tree(parent_tree, resource_element, datatypes_for_writing,
                         resource_id, DEFAULT_COMMENTS,
                         DEFAULT_TABLE_LABELED_PROPERTIES);
}


}  // namespace tablator
