#pragma once

#include "Column.hxx"
#include "Data_Details.hxx"
#include "Field_Framework.hxx"

namespace tablator {

class Data_Element {
public:
    Data_Element(const Field_Framework &field_framework,
                 const Data_Details &data_details)
            : field_framework_(field_framework), data_details_(data_details) {}

    Data_Element(const Field_Framework &field_framework, size_t num_rows = 0)
            : Data_Element(field_framework, Data_Details(field_framework, num_rows)) {}

    // accessors
    const Field_Framework &get_field_framework() const { return field_framework_; }

    Field_Framework &get_field_framework() { return field_framework_; }


    const Data_Details &get_data_details() const { return data_details_; }

    Data_Details &get_data_details() { return data_details_; }

    inline const std::vector<Column> &get_columns() const {
        return field_framework_.get_columns();
    }

    // Non-const to allow query_server to modify field_properties.
    inline std::vector<Column> &get_columns() { return field_framework_.get_columns(); }

    inline const std::vector<size_t> &get_offsets() const {
        return field_framework_.get_offsets();
    }

    inline const std::vector<std::vector<char>> &get_data() const {
        return data_details_.get_data();
    }

    // Non-const to support append_rows().
    inline std::vector<std::vector<char>> &get_data() { return data_details_.get_data(); }

    inline void set_data(const std::vector<std::vector<char>> &d) { data_details_.set_data(d); }

    size_t get_row_size() const { return field_framework_.get_row_size(); }
    size_t get_num_rows() const { return data_details_.get_num_rows(); }

    size_t get_num_dynamic_columns() const {
        return field_framework_.get_num_dynamic_columns();
    }

    // called by query_server to trim result set
    void adjust_num_rows(const size_t &new_num_rows) {
        data_details_.adjust_num_rows(new_num_rows);
    }

    void reserve_rows(const size_t &new_num_rows) {
        data_details_.reserve_rows(new_num_rows);
    }


private:
    // Non-const to allow query_server to update column's field_properties.
    Field_Framework field_framework_;

    // Non-const because of append_rows().
    Data_Details data_details_;
};

}  // namespace tablator
