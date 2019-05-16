#include "../../../../../Table.hxx"
#include "../../../skip_xml_comments.hxx"
#include "../../VOTable_Field.hxx"

#include <algorithm>

void tablator::Table::read_data(const boost::property_tree::ptree &data,
                                const std::vector<VOTable_Field> &fields) {
    auto child = data.begin();
    auto end = data.end();

    if (child == end) throw std::runtime_error("RESOURCE.TABLE.DATA must not be empty");

    child = skip_xml_comments(child, end);
    if (child->first == "TABLEDATA") {
        read_tabledata(child->second, fields);
        ++child;
    } else if (child->first == "BINARY") {
        throw std::runtime_error(
                "BINARY serialization inside a VOTable not "
                "supported.");
    } else if (child->first == "BINARY2") {
        read_binary2(child->second, fields);
        ++child;
    } else if (child->first == "FITS") {
        throw std::runtime_error(
                "FITS serialization inside a VOTable not "
                "supported.");
    } else {
        throw std::runtime_error(
                "Invalid element inside RESOURCE.TABLE.DATA.  "
                "Expected one of TABLEDATA, BINARY, BINARY2, "
                "or FITS, but got: " +
                child->first);
    }

    child = skip_xml_comments(child, end);
    while (child != end) {
        if (child->first == "INFO") {
            read_node_and_attributes("RESOURCE.TABLE.DATA.INFO", child->second);
        } else {
            throw std::runtime_error(
                    "Invalid element inside RESOURCE.TABLE.DATA.  "
                    "Expected INFO but got: " +
                    child->first);
        }
        ++child;
        child = skip_xml_comments(child, end);
    }
}
