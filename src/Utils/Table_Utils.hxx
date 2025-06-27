#pragma once

#include <set>

#include "../Column.hxx"
#include "../Field_Framework.hxx"
#include "../Row.hxx"

namespace tablator {

// column-related functions

void append_column(Field_Framework &field_framework, const Column &column);

inline void append_column(Field_Framework &field_framework, const std::string &name,
                          const Data_Type &type, const size_t &array_size,
                          const Field_Properties &field_properties,
                          bool dynamic_array_flag) {
    append_column(field_framework,
                  Column(name, type, array_size, field_properties, dynamic_array_flag));
}

inline void append_column(Field_Framework &field_framework, const std::string &name,
                          const Data_Type &type, const size_t &array_size,
                          const Field_Properties &field_properties) {
    append_column(field_framework, Column(name, type, array_size, field_properties));
}

inline void append_column(Field_Framework &field_framework, const std::string &name,
                          const Data_Type &type, const size_t &array_size,
                          bool dynamic_array_flag) {
    append_column(field_framework, Column(name, type, array_size, dynamic_array_flag));
}

inline void append_column(Field_Framework &field_framework, const std::string &name,
                          const Data_Type &type, const size_t &size) {
    append_column(field_framework, Column(name, type, size));
}

inline void append_column(Field_Framework &field_framework, const std::string &name,
                          const Data_Type &type,
                          const Field_Properties &field_properties) {
    append_column(field_framework, Column(name, type, field_properties));
}


inline void append_column(Field_Framework &field_framework, const std::string &name,
                          const Data_Type &type) {
    append_column(field_framework, Column(name, type));
}

// row-related functions

inline size_t get_row_size(const std::vector<size_t> &offsets) {
    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
    return offsets.back();
}

inline size_t get_num_rows(const std::vector<size_t> &offsets,
                           const std::vector<uint8_t> &data) {
    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
    return data.size() / get_row_size(offsets);
}
inline void append_row(std::vector<uint8_t> &data, const Row &row) {
    data.insert(data.end(), row.get_data().begin(), row.get_data().end());
}

inline void unsafe_append_row(std::vector<uint8_t> &data, const char *row,
                              uint row_size) {
    data.insert(data.end(), row, row + row_size);
}

inline void append_rows(std::vector<uint8_t> &data, const std::vector<uint8_t> &data2) {
    data.insert(data.end(), data2.begin(), data2.end());
}


inline void resize_data(std::vector<uint8_t> &data, const size_t &new_num_rows,
                        uint row_size) {
    data.resize(new_num_rows * row_size);
}

inline void reserve_data(std::vector<uint8_t> &data, const size_t &new_num_rows,
                         uint row_size) {
    data.reserve(new_num_rows * row_size);
}


void insert_ascii_in_row(Row &row, const Data_Type &data_type, const size_t &array_size,
                         const size_t &column, const std::string &element,
                         const size_t &offset, const size_t &offset_end);


void retain_only_selected_rows(std::vector<uint8_t> &data,
                               const std::set<size_t> &selected_row_idx_list,
                               size_t num_rows, uint row_size);


}  // namespace tablator
