#pragma once

#include "Column.hxx"
#include "Field_Framework.hxx"

namespace tablator {

class Data_Element {
public:
    Data_Element(const Field_Framework &field_framework, const std::vector<uint8_t> &data)
            : field_framework_(field_framework), data_(data) {}

    Data_Element(const Field_Framework &field_framework)
            : Data_Element(field_framework, std::vector<uint8_t>()) {}


    // accessors
    inline const std::vector<Column> &get_columns() const {
        return field_framework_.get_columns();
    }

    // Non-const to allow query_server to modify field_properties.
    inline std::vector<Column> &get_columns() { return field_framework_.get_columns(); }

    inline const std::vector<size_t> &get_offsets() const {
        return field_framework_.get_offsets();
    }

    inline const std::vector<uint8_t> &get_data() const { return data_; }

    // Non-const to support append_rows().
    inline std::vector<uint8_t> &get_data() { return data_; }

    inline void set_data(const std::vector<uint8_t> &d) { data_ = d; }

private:
    // Non-const to allow query_server to update column's field_properties.
    Field_Framework field_framework_;

    // Can't be const because of append_rows().
    std::vector<uint8_t> data_;
};

}  // namespace tablator
