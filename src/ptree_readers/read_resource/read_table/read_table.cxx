#include "../../../ptree_readers.hxx"

#include "../../../Property.hxx"
#include "../../../Table_Element.hxx"
#include "../../../Utils/Table_Utils.hxx"
#include "../../../Utils/Vector_Utils.hxx"

tablator::Table_Element tablator::ptree_readers::read_table(
        const boost::property_tree::ptree &table_tree) {
    auto child = table_tree.begin();
    auto end = table_tree.end();

    const auto attributes = extract_attributes(table_tree);

    while (child != end && child->first == XMLATTR) {
        ++child;
    }

    std::string description;
    if (child != end && child->first == DESCRIPTION) {
        description = child->second.get_value<std::string>();
        ++child;
    }

    std::vector<Field_And_Flag> field_flag_pairs;
    // Register null column
    field_flag_pairs.emplace_back(
            Field(null_bitfield_flags_name, Data_Type::UINT8_LE, true,
                  Field_Properties(null_bitfield_flags_description, {})),
            false);
    std::vector<Group_Element> group_elements;
    std::vector<Data_Element> data_elements;
    std::vector<Field> params;

    while (child != end) {
        if (child->first == FIELD) {
            field_flag_pairs.emplace_back(read_field(child->second));
        } else if (child->first == PARAM) {
            params.emplace_back(read_field(child->second).get_field());
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
    }

    if (field_flag_pairs.size() < 2) {
        throw std::runtime_error("This VOTable is empty.");
    }

    if (child != end && child->first == DATA) {
        data_elements.emplace_back(read_data(child->second, field_flag_pairs));
        ++child;
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
    }


    std::vector<Field> fields;
    std::transform(field_flag_pairs.begin(), field_flag_pairs.end(),
                   std::back_inserter(fields),
                   [&](const Field_And_Flag &field_flag) -> Field {
                       return field_flag.get_field();
                   });

    return tablator::Table_Element::Builder(data_elements)
            .add_attributes(attributes)
            .add_description(description)
            .add_group_elements(group_elements)
            .add_params(params)
            .add_fields(fields)
            .add_trailing_info_list(trailing_info_list)
            .build();
}
