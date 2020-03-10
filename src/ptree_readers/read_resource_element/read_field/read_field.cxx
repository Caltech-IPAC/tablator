#include "../../../ptree_readers.hxx"

#include <boost/lexical_cast.hpp>

namespace tablator {
Values read_values(const boost::property_tree::ptree &values);
}

namespace {
tablator::Data_Type string_to_Type(const std::string &s) {
    if (s == "boolean") return tablator::Data_Type::INT8_LE;
    if (s == "unsignedByte") return tablator::Data_Type::UINT8_LE;
    if (s == "short") return tablator::Data_Type::INT16_LE;
    if (s == "ushort") return tablator::Data_Type::UINT16_LE;
    if (s == "int") return tablator::Data_Type::INT32_LE;
    if (s == "uint") return tablator::Data_Type::UINT32_LE;
    if (s == "long") return tablator::Data_Type::INT64_LE;
    if (s == "ulong") return tablator::Data_Type::UINT64_LE;
    if (s == "float") return tablator::Data_Type::FLOAT32_LE;
    if (s == "double") return tablator::Data_Type::FLOAT64_LE;
    if (s == "char") return tablator::Data_Type::CHAR;
    // FIXME: Implement these
    if (s == "bit" || s == "unicodeChar" || s == "floatComplex" || s == "doubleComplex")
        throw std::runtime_error("Unimplemented data type: " + s);
    throw std::runtime_error("Unknown data type: " + s);
}
}  // namespace

tablator::ptree_readers::Field_And_Flag tablator::ptree_readers::read_field(
        const boost::property_tree::ptree &field_tree) {
    // Set default values for Field class members and adjust as we read the ptree.
    std::string name = "";
    Data_Type type = tablator::Data_Type::UINT8_LE;
    size_t array_size = 1;
    Field_Properties field_properties;

    bool is_array_dynamic_f = false;

    auto child = field_tree.begin();
    auto end = field_tree.end();

    if (child != end && child->first == XMLATTR) {
        for (auto &attribute : child->second) {
            if (attribute.first == ATTR_NAME) {
                name.assign(attribute.second.get_value<std::string>());
            } else if (attribute.first == DATATYPE) {
                type = string_to_Type(attribute.second.get_value<std::string>());

            }
            // FIXME: We do not handle arrays correctly
            else if (attribute.first == ARRAYSIZE) {
                std::string array_size_str = attribute.second.get_value<std::string>();
                if (array_size_str == "*") {
                    array_size = std::numeric_limits<size_t>::max();
                    is_array_dynamic_f = true;
                } else {
                    array_size = boost::lexical_cast<size_t>(array_size_str);
                }
            } else {
                field_properties.add_attribute(
                        attribute.first, attribute.second.get_value<std::string>());
            }
        }
        ++child;
    }

    if (child != end && child->first == DESCRIPTION) {
        field_properties.set_description(child->second.get_value<std::string>());
        ++child;
    }
    if (child != end && child->first == VALUES) {
        field_properties.set_values(read_values(child->second));
        ++child;
    }
    if (child != end && child->first == LINK) {
        for (auto &link_child : child->second) {
            if (link_child.first == XMLATTR) {
                for (auto &attribute : link_child.second) {
                    field_properties.add_link(
                            attribute.first, attribute.second.get_value<std::string>());
                }
            } else {
                throw std::runtime_error(
                        "Expected only attributes inside "
                        "LINK elements, but found: " +
                        link_child.first);
            }
        }
        ++child;
    }
    return Field_And_Flag(Field(name, type, array_size, field_properties),
                          is_array_dynamic_f);
}
