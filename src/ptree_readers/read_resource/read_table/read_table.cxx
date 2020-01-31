#include "../../../ptree_readers.hxx"

#include "../../../Utils/Vector_Utils.hxx"
#include "../VOTable_Field.hxx"

tablator::Table_Element tablator::ptree_readers::read_table(
        const boost::property_tree::ptree &table_tree) {
    auto child = table_tree.begin();
    auto end = table_tree.end();

    const auto attributes = extract_attributes(table_tree);
    child = skip_xml_comments(child, end);

    while (child != end && child->first == XMLATTR) {
        ++child;
        child = skip_xml_comments(child, end);
    }
    child = skip_xml_comments(child, end);

    std::string description;
    if (child != end && child->first == DESCRIPTION) {
        description = child->second.get_value<std::string>();
        ++child;
    }
    child = skip_xml_comments(child, end);

    std::vector<VOTable_Field> fields;
    fields.emplace_back(null_bitfield_flags_name, Data_Type::UINT8_LE, true,
                        Field_Properties(null_bitfield_flags_description, {}));

    std::vector<Group_Element> group_elements;
    std::vector<Data_Element> data_elements;
    std::vector<Column> params;

    while (child != end) {
        if (child->first == FIELD) {
            fields.emplace_back(read_field(child->second));
        } else if (child->first == PARAM) {
            params.emplace_back(read_field(child->second));
        } else if (child->first == GROUP) {
            group_elements.emplace_back(read_group(child->second));
        } else if ((child->first == DATA) || (child->first == INFO)) {
            break;
        } else {
            throw std::runtime_error(
                    "In table element, expected FIELD, PARAM, GROUP, DATA, or INFO, "
                    "but found " +
                    child->first);
        }
        ++child;
        child = skip_xml_comments(child, end);
    }

    if (fields.size() < 2) {
        throw std::runtime_error("This VOTable is empty.");
    }

    if (child != end && child->first == DATA) {
        data_elements.emplace_back(read_data(child->second, fields));
        ++child;
        child = skip_xml_comments(child, end);
    }

    std::vector<Property> trailing_info_list;
    while (child != end) {
        if (child->first == INFO) {
            trailing_info_list.emplace_back(read_property(child->second));
        } else {
            throw std::runtime_error(std::string("In table element, expected INFO tag, "
                                                 "if anything, but found ") +
                                     child->first);
        }
        ++child;
        child = skip_xml_comments(child, end);
    }


    // Nobody from now on cares about VOTable_Fields.  Convert to Columns.
    std::vector<Column> field_columns;
    field_columns.insert(field_columns.end(), fields.begin(), fields.end());

    return tablator::Table_Element::Builder(data_elements)
            .add_attributes(attributes)
            .add_description(description)
            .add_group_elements(group_elements)
            .add_params(params)
            .add_fields(field_columns)
            .add_trailing_info_list(trailing_info_list)
            .build();
}
