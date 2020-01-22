#include <boost/property_tree/ptree.hpp>
#include "../../Table.hxx"
#include "../../to_xml_string.hxx"

namespace {
void Min_Max_to_xml(boost::property_tree::ptree &tree, const std::string &min_max,
                    const tablator::Min_Max &m) {
    if (!m.empty()) {
        boost::property_tree::ptree &min_max_tree = tree.add(min_max, "");
        min_max_tree.add("<xmlattr>.value", m.value);
        min_max_tree.add("<xmlattr>.inclusive", m.inclusive ? "yes" : "no");
    }
}

void Option_to_xml(boost::property_tree::ptree &tree, const tablator::Option &option) {
    if (!option.empty()) {
        boost::property_tree::ptree &option_tree = tree.add("OPTION", "");
        if (!option.name.empty()) {
            option_tree.add("<xmlattr>.name", option.name);
        }
        if (!option.value.empty()) {
            option_tree.add("<xmlattr>.value", option.value);
        }
        for (auto &o : option.options) Option_to_xml(option_tree, o);
    }
}
}  // namespace

namespace tablator {
void add_to_property_tree(const Column &column, const std::string &tree_name,
                          boost::property_tree::ptree &tree,
                          const Data_Type &active_datatype) {
    boost::property_tree::ptree &field = tree.add(tree_name, "");
    field.add("<xmlattr>.name", column.name);
    std::string datatype = to_xml_string(active_datatype);
    field.add("<xmlattr>.datatype", datatype);

    bool added_arraysize =
            (active_datatype == Data_Type::CHAR || column.array_size != 1);
    if (added_arraysize) {
        field.add("<xmlattr>.arraysize", "*");
    }
    for (auto &a : column.field_properties.attributes) {
        // Empty attributes cause field.add to crash :(, so make sure
        // that does not happen.
        // FIXME: This error is thrown a bit too late to be useful.
        if (a.first.empty())
            throw std::runtime_error("Empty attribute in field " + column.name +
                                     " which has type " + to_string(column.type));

        if (added_arraysize && boost::equals(a.first, "arraysize")) {
            continue;
        }

        field.add("<xmlattr>." + a.first, a.second);
    }

    if (!column.field_properties.description.empty())
        field.add("DESCRIPTION", column.field_properties.description);

    auto &v(column.field_properties.values);
    if (!v.empty_except_null()) {
        boost::property_tree::ptree &values = field.add("VALUES", "");
        if (!v.ID.empty()) values.add("<xmlattr>.ID", v.ID);
        if (!v.type.empty()) values.add("<xmlattr>.type", v.type);
        if (!v.ref.empty()) values.add("<xmlattr>.ref", v.ref);

        Min_Max_to_xml(values, "MIN", v.min);
        Min_Max_to_xml(values, "MAX", v.max);

        for (auto &o : v.options) Option_to_xml(values, o);
    }

    if (!column.field_properties.links.empty()) {
        boost::property_tree::ptree &link = field.add("LINK", "");
        for (auto &l : column.field_properties.links)
            link.add("<xmlattr>." + l.first, l.second);
    }
}
}  // namespace tablator
