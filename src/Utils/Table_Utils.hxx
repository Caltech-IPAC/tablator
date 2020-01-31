#pragma once

#include "../Column.hxx"
#include "../Row.hxx"

namespace tablator {

    static const std::string null_bitfield_flags_name("null_bitfield_flags");
    static const std::string null_bitfield_flags_description(
        "Packed bit array indicating whether an entry is null");

    inline  size_t row_size(const std::vector<size_t> &offsets) {
        if (offsets.empty()) {
            throw std::runtime_error("<offsets> is empty");
        }
        return offsets.back();
    }

    void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const Column &column);

    inline void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const std::string &name,
                              const Data_Type &type, const size_t &size,
                              const Field_Properties &field_properties) {
        append_column(columns, offsets, Column(name, type, size, field_properties));
    }

    inline void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const std::string &name,
                              const Data_Type &type, const size_t &size) {
        append_column(columns, offsets, Column(name, type, size, Field_Properties()));
    }

    inline void append_column(std::vector<Column> &columns,
                              std::vector<size_t> &offsets, const std::string &name,
                              const Data_Type &type) {
        append_column(columns, offsets, name, type, 1, Field_Properties());
    }

    inline void append_row(std::vector<uint8_t> &data, const Row &row) {
        data.insert(data.end(), row.data.begin(), row.data.end());
    }

    inline void unsafe_append_row(std::vector<uint8_t> &data, const char *row,
                                  uint row_size) {
        data.insert(data.end(), row, row + row_size);
    }

    inline void pop_row(std::vector<uint8_t> &data, uint row_size) {
        data.resize(data.size() - row_size);
    }

    inline void resize_rows(std::vector<uint8_t> &data, const size_t &new_num_rows,
                            uint row_size) {
        data.resize(new_num_rows * row_size);
    }

    void insert_ascii_in_row(const Data_Type &data_type, const size_t &array_size,
                             const size_t &column, const std::string &element,
                             const size_t &offset, const size_t &offset_end, Row &row);

} // namespace tablator
