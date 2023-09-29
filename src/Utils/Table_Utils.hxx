#pragma once

#include "../Column.hxx"
#include "../Data_Type.hxx"
#include "../Row.hxx"

namespace tablator {

void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                   const Column &column);

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type, size_t size,
                          const Field_Properties &field_properties) {
    append_column(columns, offsets, Column(name, type, size, field_properties));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type, size_t size) {
    append_column(columns, offsets, Column(name, type, size, Field_Properties()));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type) {
    append_column(columns, offsets, name, type, 1, Field_Properties());
}

// This converts from uint8_t to char.  :-(
inline void append_row(std::vector<uint8_t> &data, const Row &row) {
    data.insert(data.end(), row.data_.begin(), row.data_.end());
}

inline void unsafe_append_row(std::vector<uint8_t> &data, const char *row,
                              uint row_size) {
    data.insert(data.end(), row, row + row_size);
}

inline void pop_row(std::vector<uint8_t> &data, uint row_size) {
    data.resize(data.size() - row_size);
}

inline void resize_rows(std::vector<uint8_t> &data, size_t new_num_rows,
                        uint row_size) {
    data.resize(new_num_rows * row_size);
}

void insert_ascii_in_row(const Data_Type &data_type, size_t array_size, size_t col_idx,
                         const std::string &element, size_t offset, size_t offset_end,
                         Row &row);

}  // namespace tablator
