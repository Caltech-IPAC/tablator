#pragma once

#include <set>

#include "../Column.hxx"
#include "../Row.hxx"

#include "../Utils/Table_Utils.hxx"

namespace tablator {

// column-related functions
inline size_t get_col_data_start_offset(const std::vector<size_t> &offsets,
                                        size_t col_idx, bool dynamic_array_flag) {
    size_t col_offset = offsets[col_idx];
    if (dynamic_array_flag) {
        return col_offset + DYNAMIC_ARRAY_OFFSET;
    }
    return col_offset;
}

inline size_t get_col_start_offset(const std::vector<size_t> &offsets, size_t col_idx) {
    assert(col_idx < offsets.size());
    return offsets[col_idx];
}


inline size_t get_col_end_offset(const std::vector<size_t> &offsets, size_t col_idx) {
    assert(col_idx < offsets.size() - 1);
    return offsets[col_idx + 1];
}

void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                   const Column &column);


inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type,
                          const size_t &size, const Field_Properties &field_properties,
                          bool dynamic_array_flag) {
    append_column(columns, offsets,
                  Column(name, type, size, field_properties, dynamic_array_flag));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type,
                          const size_t &size,
                          const Field_Properties &field_properties) {
    append_column(
            columns, offsets,
            Column(name, type, size, field_properties, false /* dynamic_array_flag */));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type,
                          const size_t &size, bool dynamic_array_flag) {
    append_column(columns, offsets,
                  Column(name, type, size, Field_Properties(), dynamic_array_flag));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type,
                          const size_t &size) {
    append_column(columns, offsets, Column(name, type, size, Field_Properties()));
}


inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type) {
    append_column(columns, offsets, name, type, 1, Field_Properties());
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
                         const size_t &offset, const size_t &offset_end,
                         bool dynamic_array_flag);


// JTODO move to .cxx file
inline void retain_only_selected_rows(std::vector<uint8_t> &data,
                                      const std::set<size_t> &selected_row_idx_list,
                                      size_t num_rows, uint row_size) {
    if (data.size() != num_rows * row_size) {
        // JTODO relax this condition?
        throw std::runtime_error(
                "Mismatch between data.size(), num_rows, and row_size.");
    }

    size_t num_selected_rows = selected_row_idx_list.size();
    if (num_selected_rows > num_rows) {
        throw std::runtime_error("Number of selected rows must not exceed " +
                                 std::to_string(num_rows));
    }

    if (*selected_row_idx_list.rbegin() >= num_rows) {
        throw std::runtime_error("invalid row index: " +
                                 std::to_string(*selected_row_idx_list.rbegin()));
    }


    if (num_selected_rows == num_rows) {
        return;
    }

    const auto data_start_ptr = data.data();
    auto read_ptr = data_start_ptr;
    auto write_ptr = data_start_ptr;
    size_t write_idx = 0;
    size_t prev_row_idx = 0;

    for (auto row_idx : selected_row_idx_list) {
        if (row_idx >= write_idx) {
            if (row_idx > write_idx) {
                // Copy row_idx-th row to begin immediately after the end of the
                // previous copied row.
                read_ptr += ((row_idx - prev_row_idx) * row_size);
                std::copy(read_ptr, read_ptr + row_size, write_ptr);
            }
            ++write_idx;
            write_ptr += row_size;
            prev_row_idx = row_idx;
        }
    }

    // Delete all data past the last row copied.
    data.resize(std::distance(data_start_ptr, write_ptr));
}

}  // namespace tablator
