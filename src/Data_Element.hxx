#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Column.hxx"
#include "Utils/Table_Utils.hxx"  // JTODO

// JTODO: Create struct to hold commonly used (columns, offsets) pair?

namespace tablator {

class Data_Element {
public:
    Data_Element(const std::vector<Column> &columns, const std::vector<size_t> &offsets,
                 const std::vector<uint8_t> &data)
            : columns_(columns), offsets_(offsets), data_(data) {}

    // accessors
    inline const std::vector<Column> &get_columns() const { return columns_; }
    inline std::vector<Column> &get_columns() { return columns_; }

    inline const std::vector<size_t> &get_offsets() const { return offsets_; }
    inline std::vector<size_t> &get_offsets() { return offsets_; }


    inline const std::vector<uint8_t> &get_data() const { return data_; }
    inline std::vector<uint8_t> &get_data() { return data_; }

    inline void set_data(const std::vector<uint8_t> &d) { data_ = d; }

private:
    std::vector<Column> columns_;
    std::vector<size_t> offsets_ = {0};
    std::vector<uint8_t> data_;
};

}  // namespace tablator
