#include "../../../../ptree_readers.hxx"

#include <algorithm>

void validate_tail(boost::property_tree::ptree::const_iterator &tail_begin,
                   boost::property_tree::ptree::const_iterator &tail_end) {
    boost::property_tree::ptree::const_iterator child = tail_begin;
    if (child != tail_end) {
        throw std::runtime_error("Unexpected element " + child->first +
                                 " at end of RESOURCE.TABLE.DATA.");
    }
}

//=================================================================

tablator::Data_Element tablator::ptree_readers::read_data(
        const boost::property_tree::ptree &data,
        const std::vector<ptree_readers::Field_And_Flag> &field_flag_pairs) {
    auto child = data.begin();
    auto end = data.end();

    if (child == end) {
        throw std::runtime_error("RESOURCE.TABLE.DATA must not be empty");
    }

    if (child->first == TABLEDATA) {
        auto result = read_tabledata(child->second, field_flag_pairs);
        validate_tail(++child, end);
        return result;
    } else if (child->first == BINARY) {
        throw std::runtime_error(
                "BINARY serialization inside a VOTable not "
                "supported.");
    } else if (child->first == BINARY2) {
        auto result = read_binary2(child->second, field_flag_pairs);
        validate_tail(++child, end);
        return result;
    } else if (child->first == FITS) {
        throw std::runtime_error(
                "FITS serialization inside a VOTable not "
                "supported.");
    }
    throw std::runtime_error(
            "Invalid first non-comment element inside RESOURCE.TABLE.DATA. "
            "Expected one of TABLEDATA, BINARY, BINARY2, "
            "or FITS, but got: " +
            child->first);
}
