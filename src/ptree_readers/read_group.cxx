#include "../ptree_readers.hxx"

#include "../Column.hxx"
#include "../Common.hxx"
#include "read_resource/VOTable_Field.hxx"  // JTODO

tablator::Group_Element tablator::ptree_readers::read_group(
        const boost::property_tree::ptree &group_tree) {
    auto child = group_tree.begin();
    auto end = group_tree.end();

    const auto &attributes = extract_attributes(group_tree);
    child = ptree_readers::skip_xml_comments(child, end);

    // JTODO Assume XMLATTRs are all at front?
    while (child != end && child->first == XMLATTR) {
        ++child;
        child = ptree_readers::skip_xml_comments(child, end);
    }

    std::string description;
    if (child != end && child->first == DESCRIPTION) {
        description = child->second.get_value<std::string>();
        ++child;
    }

    std::vector<Column> params;
    std::vector<ATTRIBUTES> field_refs;
    std::vector<ATTRIBUTES> param_refs;
    while (child != end) {
        if (child->first == "FIELDref") {
            field_refs.emplace_back(extract_attributes(child->second));
        } else if (child->first == "PARAMref") {
            param_refs.emplace_back(extract_attributes(child->second));
        } else if (child->first == "PARAM") {
            // JTODO this function returns VOTable_Field; all we need is Column.
            params.emplace_back(read_field(child->second));
        } else if (child->first == "GROUP") {
            //            read_group("GROUP", child->second);  // JTODO recurse
        } else {
            break;
        }
        ++child;
        child = ptree_readers::skip_xml_comments(child, end);
    }
    return Group_Element::Builder()
            .add_attributes(attributes)
            .add_description(description)
            .add_params(params)
            .add_field_refs(field_refs)
            .build();
}
