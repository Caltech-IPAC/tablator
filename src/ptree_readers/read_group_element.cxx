#include "../ptree_readers.hxx"

#include "../Common.hxx"
#include "../Group_Element.hxx"

tablator::Group_Element tablator::ptree_readers::read_group_element(
        const boost::property_tree::ptree &group_tree) {
    auto child = group_tree.begin();
    auto end = group_tree.end();

    const auto &attributes = extract_attributes(group_tree);

    // JTODO Assume XMLATTRs are all at front?
    while (child != end && child->first == XMLATTR) {
        ++child;
    }

    std::string description;
    if (child != end && child->first == DESCRIPTION) {
        description = child->second.get_value<std::string>();
        ++child;
    }

    std::vector<Field> params;
    std::vector<ATTRIBUTES> field_refs;
    std::vector<ATTRIBUTES> param_refs;
    while (child != end) {
        if (child->first == FIELDREF) {
            field_refs.emplace_back(extract_attributes(child->second));
        } else if (child->first == PARAMREF) {
            param_refs.emplace_back(extract_attributes(child->second));
        } else if (child->first == PARAM) {
            params.emplace_back(read_field(child->second).get_field());
        } else if (child->first == GROUP) {
            //            read_group_element(GROUP, child->second);  // JTODO recurse
        } else {
            break;
        }
        ++child;
    }
    return Group_Element::Builder()
            .add_attributes(attributes)
            .add_description(description)
            .add_params(params)
            .add_field_refs(field_refs)
            .build();
}
