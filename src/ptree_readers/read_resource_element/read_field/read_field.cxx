#include "../../../ptree_readers.hxx"

#include <boost/algorithm/string/predicate.hpp>
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


// Unlike other classes with "links" elements, Field_Properties has
// both "links" and "hdf5_links", which are both represented in
// VOTables as LINK elements and must be disentangled here.

void load_link_singleton(tablator::Labeled_Properties &labeled_properties,
                         std::vector<tablator::STRING_PAIR> &hdf5_links,
                         const boost::property_tree::ptree &node) {
    const auto attrs = tablator::ptree_readers::extract_attributes(node);

    if (attrs.size() == 1) {
        const auto &name = attrs.begin()->first;
        if (boost::starts_with(name, tablator::HDF5_LINK)) {
            hdf5_links.emplace_back(std::make_pair(
                    name.substr(tablator::HDF5_LINK.size()), attrs.begin()->second));
            return;
        }
    }

    labeled_properties.emplace_back(tablator::LINK, tablator::Property(attrs));
}

void load_link_array(tablator::Labeled_Properties &labeled_properties,
                     std::vector<tablator::STRING_PAIR> &hdf5_links,
                     const boost::property_tree::ptree &array_tree) {
    for (const auto &elt : array_tree) {
        load_link_singleton(labeled_properties, hdf5_links, elt.second);
    }
}


boost::property_tree::ptree::const_iterator read_links_section(
        tablator::Labeled_Properties &links,
        std::vector<tablator::STRING_PAIR> &hdf5_links,
        boost::property_tree::ptree::const_iterator &start,
        boost::property_tree::ptree::const_iterator &end, const std::string &next_tag) {
    boost::property_tree::ptree::const_iterator &iter = start;

    while (iter != end) {
        if (iter->first == tablator::LINK) {
            load_link_singleton(links, hdf5_links, iter->second);
        } else if (iter->first == tablator::LINK_ARRAY) {
            load_link_array(links, hdf5_links, iter->second);

        } else if (iter->first == next_tag) {
            break;
        } else {
            throw std::runtime_error(std::string("Expected LINK or TABLE tag, "
                                                 "but found ") +
                                     iter->first);
        }
        ++iter;
    }
    return iter;
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

    Labeled_Properties &links = field_properties.get_links();
    std::vector<STRING_PAIR> &hdf5_links = field_properties.get_hdf5_links();
    child = read_links_section(links, hdf5_links, child, end, TABLE);

    return Field_And_Flag(Field(name, type, array_size, field_properties),
                          is_array_dynamic_f);
}
