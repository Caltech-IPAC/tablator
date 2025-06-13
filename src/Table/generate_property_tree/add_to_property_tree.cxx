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
        min_max_tree.add(tablator::XMLATTR_VALUE, m.value);
        min_max_tree.add(tablator::XMLATTR_INCLUSIVE, m.inclusive ? "yes" : "no");
    }
}

// If json_prep is true, find (or, if none exists, create) a tree with
// label <label>_ARRAY, add an un-labeled subtree to that, and return
// the subtree.  Otherwise, add and return a subtree with label
// <label> whether or not a subtree already exists with that label.
boost::property_tree::ptree &find_or_add_tree(boost::property_tree::ptree &parent_tree,
                                              const std::string &label,
                                              bool json_prep) {
    if (!json_prep) {
        return parent_tree.add(label, "");
    }

    boost::optional<boost::property_tree::ptree &> array_tree_opt =
            parent_tree.get_child_optional(label + tablator::ARRAY_TAIL);
    boost::property_tree::ptree &array_tree =
            array_tree_opt ? array_tree_opt.get()
                           : parent_tree.add(label + tablator::ARRAY_TAIL, "");
    array_tree.push_back(std::make_pair("", boost::property_tree::ptree()));
    boost::property_tree::ptree &new_tree = array_tree.back().second;
    return new_tree;
}

}  // namespace

namespace tablator {

//==============================================================
//*** Option
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Option &option, bool json_prep) {
    if (!option.empty()) {
        auto &option_tree = find_or_add_tree(parent_tree, "OPTION", json_prep);
        if (!option.name.empty()) {
            option_tree.add(tablator::XMLATTR_NAME, option.name);
        }
        if (!option.value.empty()) {
            option_tree.add(tablator::XMLATTR_VALUE, option.value);
        }

        for (auto &suboption : option.options) {
            add_to_property_tree(option_tree, suboption, json_prep);
        }
    }
}

//==============================================================
//*** ATTRIBUTES
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const tablator::ATTRIBUTES &attributes) {
    // std::cout << "add_to_property_tree(), Attributes" << std::endl;
    for (const auto &attr_pair : attributes) {
        parent_tree.add(tablator::XMLATTR_DOT + attr_pair.first, attr_pair.second);
    }
}

//==============================================================
//*** Property
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &label, const Property &property,
                          bool json_prep) {
    // std::cout << "add_to_property_tree(), Prop" << std::endl;
    const auto &value = property.get_value();
    const auto &attributes = property.get_attributes();

    const auto name_iter = attributes.find(ATTR_NAME);
    const auto value_iter = attributes.find(ATTR_VALUE);

    if (label == tablator::INFO && attributes.size() == 2 &&
        name_iter != attributes.end() && value_iter != attributes.end()) {
        // Check for one of the few supported Resource attributes as encoded by
        // Ipac_Table_Writer.

        // JTODO: Check that we're at resource level?
        const std::string &name = name_iter->second;
        if (boost::iequals(name, std::string(tablator::TYPE)) ||
            boost::iequals(name, std::string(tablator::UTYPE)) ||
            boost::iequals(name, std::string(tablator::NAME)) ||
            boost::iequals(name, std::string(tablator::ID))) {
            tablator::ATTRIBUTES attrs{{name, value_iter->second}};
            add_to_property_tree(parent_tree, {{name, value_iter->second}});
            return;
        }
    }

    if (is_property_style_label(label)) {
        // std::cout << "is_property_stype_label()" << std::endl;
        auto &label_tree = find_or_add_tree(parent_tree, label, json_prep);
        if (!value.empty()) {
            if (json_prep) {
                // value must be stored with a label for JSON format
                label_tree.add(tablator::VALUE, value);
            } else {
                // VOTABLE format expects a property_tree-style value
                label_tree.put_value(value);
            }
        }
        add_to_property_tree(label_tree, attributes);
    } else if (boost::starts_with(label, XMLATTR)) {
        // std::cout << "label starts with XMLATTR" << std::endl;
        parent_tree.add(label, value);
    } else {
        // For write_fits()
        // std::cout << "creating INFO" << std::endl;
        auto &label_tree = parent_tree.add(INFO, "");
        label_tree.add(XMLATTR_NAME, label);
        if (!value.empty()) {
            label_tree.add(XMLATTR_VALUE, value);
        }
        add_to_property_tree(label_tree, attributes);
    }
}

//==============================================================
//*** Labeled_Properties
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Labeled_Properties &labeled_properties,
                          bool json_prep) {
    // std::cout << "add_to_property_tree(), Labeled_Props" << std::endl;
    for (const auto &labeled_list : labeled_properties) {
        add_to_property_tree(parent_tree, labeled_list.first, labeled_list.second,
                             json_prep);
    }
}


//==============================================================
//*** Column
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &col_label, const Column &column,
                          const Data_Type &active_datatype, bool json_prep) {
    auto &field_tree = find_or_add_tree(parent_tree, col_label, json_prep);

    // Attributes first, starting with those that do not (ordinarily)
    // come from column.field_properties.attributes, namely name,
    // datatype, and arraysize.
    field_tree.add(XMLATTR_NAME, column.get_name());

    std::string datatype = to_xml_string(active_datatype);
    field_tree.add(XMLATTR_DATATYPE, datatype);

    size_t col_array_size = column.get_array_size();
    bool early_arraysize_f = false;

    if (active_datatype == Data_Type::CHAR && column.get_type() != Data_Type::CHAR) {
        field_tree.add(XMLATTR_ARRAYSIZE, "*");
        early_arraysize_f = true;
    } else if (column.get_dynamic_array_flag()) {
        field_tree.add(XMLATTR_ARRAYSIZE, "*");
        early_arraysize_f = true;
    } else if (col_array_size != 1) {
        // VOTable spec says not to show arraysize when value == 1.
        field_tree.add(XMLATTR_ARRAYSIZE,
                       (col_array_size == std::numeric_limits<size_t>::max())
                               ? "*"
                               : std::to_string(col_array_size));
        early_arraysize_f = true;
    }

    // On to attributes from field_properties.attributes.
    const auto &field_properties = column.get_field_properties();
    for (auto &a : field_properties.get_attributes()) {
        // Empty attributes cause field_tree.add to crash :(, so make sure
        // that does not happen.

        // FIXME: This error is thrown a bit too late to be useful.
        if (a.first.empty()) {
            throw std::runtime_error("Empty attribute in field " + column.get_name() +
                                     " which has type " + to_string(column.get_type()));
        }
        if (boost::equals(a.first, "arraysize")) {
            if (early_arraysize_f) {
                continue;
            }
            // For certain endpoints (e.g. SSA and SIA), query_server
            // sends arraysize as an attribute.  This is helpful when
            // the result set is empty, in which case the column might
            // have been constructed with arraysize == 1 as a default.
            // JTODO handle this case better in query_server.
            if (a.second == "1") {
                continue;
            }
        }
        field_tree.add(XMLATTR_DOT + a.first, a.second);
    }

    const auto &desc = field_properties.get_description();
    if (!desc.empty()) {
        field_tree.add(DESCRIPTION, desc);
    }

    auto &values(field_properties.get_values());
    if (!values.empty_except_null()) {
        boost::property_tree::ptree &values_tree = field_tree.add("VALUES", "");
        if (!values.ID.empty()) values_tree.add(XMLATTR_ID, values.ID);
        if (!values.type.empty()) values_tree.add(XMLATTR_TYPE, values.type);
        if (!values.ref.empty()) values_tree.add(XMLATTR_REF, values.ref);

        Min_Max_to_xml(values_tree, "MIN", values.min);
        Min_Max_to_xml(values_tree, "MAX", values.max);

        for (auto &option : values.options) {
            add_to_property_tree(values_tree, option, json_prep);
        }
    }

    if (!field_properties.get_hdf5_links().empty()) {
        auto &link_tree = find_or_add_tree(field_tree, LINK, json_prep);
        for (auto &link : field_properties.get_hdf5_links()) {
            link_tree.add(XMLATTR_DOT + "hdf5.link." + link.first, link.second);
        }
    }

    // Add as Labeled_Properties
    if (!field_properties.get_links().empty()) {
        add_to_property_tree(field_tree, field_properties.get_links(), json_prep);
    }
}

//==============================================================

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const std::string &col_label, const Column &column,
                          bool json_prep) {
    add_to_property_tree(parent_tree, col_label, column, column.get_type(), json_prep);
}


//==============================================================
//*** Group_Element
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Group_Element &group_element, bool json_prep) {
    auto &group_tree = find_or_add_tree(parent_tree, GROUP, json_prep);

    for (const auto &pair : group_element.get_attributes()) {
        group_tree.add(XMLATTR_DOT + pair.first, pair.second);
    }
    if (!group_element.get_description().empty()) {
        group_tree.add(DESCRIPTION, group_element.get_description());
    }

    for (const auto &param : group_element.get_params()) {
        add_to_property_tree(group_tree, PARAM, param, json_prep);
    }

    for (const auto &attr_map : group_element.get_field_refs()) {
        boost::property_tree::ptree &field_ref_tree = group_tree.add(FIELDREF, "");
        for (const auto &attr : attr_map) {
            field_ref_tree.add(XMLATTR_DOT + attr.first, attr.second);
        }
    }
}


//==============================================================
//*** Table_Element
void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Table_Element &table_element,
                          const std::vector<Data_Type> &datatypes_for_writing,
                          const std::vector<std::string> &comments, bool json_prep) {
    // std::cout << "add_to_property_tree(), Table" << std::endl;
    boost::property_tree::ptree &table_tree = parent_tree.add(TABLE, "");

    for (const auto &pair : table_element.get_attributes()) {
        table_tree.add(XMLATTR_DOT + pair.first, pair.second);
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
        add_to_property_tree(table_tree, group, json_prep);
    }

    for (auto &param : table_element.get_params()) {
        add_to_property_tree(table_tree, PARAM, param, json_prep);
    }

    const auto columns = table_element.get_columns();

    // Skip null_bitfield_flag
    for (size_t i = 1; i < columns.size(); ++i) {
        add_to_property_tree(table_tree, FIELD, columns[i], datatypes_for_writing[i],
                             json_prep);
    }

    // We don't add tabledata directly because it isn't in ptree format.
	table_tree.add(DATA_TABLEDATA, TABLEDATA_PLACEHOLDER);

    for (const auto &info : table_element.get_trailing_info_list()) {
        add_to_property_tree(table_tree, INFO, info, json_prep);
    }
}

//==============================================================
//*** Resource_Element

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Resource_Element &resource_element,
                          const std::vector<Data_Type> &datatypes_for_writing,
                          const std::vector<std::string> &comments,
                          const Labeled_Properties &table_labeled_properties, bool json_prep) {
    // std::cout << "add_to_property_tree(), Resource I" << std::endl;
    auto &resource_tree = find_or_add_tree(parent_tree, RESOURCE, json_prep);

    for (const auto &pair : resource_element.get_attributes()) {
        resource_tree.add(XMLATTR_DOT + pair.first, pair.second);
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
                                 name.substr(VOTABLE_RESOURCE_DOT.size() + 1), prop,
                                 json_prep);
        }
    }

    if (!resource_element.get_labeled_properties().empty()) {
        add_to_property_tree(resource_tree, resource_element.get_labeled_properties(),
                             json_prep);
    }

    for (const auto &group : resource_element.get_group_elements()) {
        add_to_property_tree(resource_tree, group, json_prep);
    }

    for (const auto &param : resource_element.get_params()) {
        add_to_property_tree(resource_tree, PARAM, param, json_prep);
    }

    if (resource_element.is_results_resource()) {
        // write only one table_element
        const auto &table_element = resource_element.get_main_table_element();
        add_to_property_tree(resource_tree, table_element, datatypes_for_writing,
                             comments, json_prep);
    }

    for (const auto &info : resource_element.get_trailing_info_list()) {
        add_to_property_tree(resource_tree, INFO, info, json_prep);
    }
}


//==============================================================
//*** Resource_Element

static const std::vector<std::string> DEFAULT_COMMENTS = std::vector<std::string>();
static const Labeled_Properties DEFAULT_TABLE_LABELED_PROPERTIES = Labeled_Properties();

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Resource_Element &resource_element,
                          const std::vector<Data_Type> &datatypes_for_writing,
                          bool json_prep) {
    // std::cout << "add_to_property_tree(), Resource II" << std::endl;
    add_to_property_tree(parent_tree, resource_element, datatypes_for_writing,
                         DEFAULT_COMMENTS, DEFAULT_TABLE_LABELED_PROPERTIES, json_prep);
}

void add_to_property_tree(boost::property_tree::ptree &parent_tree,
                          const Resource_Element &resource_element, bool json_prep) {
    add_to_property_tree(parent_tree, resource_element, {} /* datatypes_for_writing */,
                         DEFAULT_COMMENTS, DEFAULT_TABLE_LABELED_PROPERTIES, json_prep);
}


}  // namespace tablator
