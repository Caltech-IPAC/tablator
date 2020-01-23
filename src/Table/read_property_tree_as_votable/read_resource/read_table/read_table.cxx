#include "../../../../Table.hxx"
#include "../../skip_xml_comments.hxx"
#include "../VOTable_Field.hxx"

void tablator::Table::read_table(const boost::property_tree::ptree &table) {
    auto child = table.begin();
    auto end = table.end();

    read_node_and_attributes("RESOURCE.TABLE", table);
    child = skip_xml_comments(child, end);
    while (child != end && child->first == "<xmlattr>") {
        ++child;
        child = skip_xml_comments(child, end);
    }
    if (child != end && child->first == "DESCRIPTION") {
        add_labeled_property(std::make_pair("RESOURCE.TABLE.DESCRIPTION",
                                            child->second.get_value<std::string>()));
        ++child;
    }
    child = skip_xml_comments(child, end);
    while (child != end && child->first == "INFO") {
        read_node_and_attributes("RESOURCE.TABLE.INFO", child->second);
        ++child;
        child = skip_xml_comments(child, end);
    }

    std::vector<VOTable_Field> fields;
    fields.emplace_back(null_bitfield_flags_name, Data_Type::UINT8_LE, true,
                        Field_Properties(null_bitfield_flags_description, {}));
    while (child != end) {
        if (child->first == "FIELD") {
            fields.emplace_back(read_field(child->second));
        } else if (child->first == "PARAM") {
            add_table_element_param(read_field(child->second));
        } else if (child->first == "GROUP") {
            // FIXME: Implement groups
        } else {
            break;
        }
        ++child;
        child = skip_xml_comments(child, end);
    }
    if (fields.size() < 2) {
        throw std::runtime_error("This VOTable is empty.");
    }
    if (child != end && child->first == "DATA") {
        read_data(child->second, fields);
        ++child;
        child = skip_xml_comments(child, end);
    }
    while (child != end && child->first == "INFO") {
        read_node_and_attributes("RESOURCE.TABLE.INFO", child->second);
        ++child;
        child = skip_xml_comments(child, end);
    }
}
