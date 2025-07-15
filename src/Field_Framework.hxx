#pragma once

#include <unordered_map>

#include "Column.hxx"
#include "Utils/Null_Utils.hxx"

namespace tablator {

class Field_Framework {
public:
    Field_Framework(const std::vector<Column> &incoming_columns,
                    bool got_null_bitfields_column = false)
            : offsets_({0}) {
        if ((got_null_bitfields_column && incoming_columns.size() == 1) ||
            incoming_columns.empty()) {
            throw std::runtime_error(
                    "Field_Framework constructor: <visible_columns> must be "
                    "non-empty.");
        }
        // JTODO avoid copies
        if (!got_null_bitfields_column) {
            const size_t null_flags_size = bits_to_bytes(incoming_columns.size() + 1);
            append_column(null_bitfield_flags_name, Data_Type::UINT8_LE,
                          null_flags_size,
                          Field_Properties::Builder()
                                  .add_description(null_bitfield_flags_description)
                                  .build(),
                          false /* dynamic_array_flag */);
        }
        for (auto &col : incoming_columns) {
            append_column(col);
        }
    }

    // accessors

    inline size_t get_row_size() const {
        if (offsets_.empty()) {
            throw std::runtime_error("<offsets> is empty");
        }
        return offsets_.back();
    }

    inline const std::vector<Column> &get_columns() const { return columns_; }
    inline std::vector<Column> &get_columns() { return columns_; }

    inline const std::vector<size_t> &get_offsets() const { return offsets_; }
    inline std::vector<size_t> &get_offsets() { return offsets_; }

    inline const std::unordered_map<size_t, size_t> &get_dynamic_col_idx_lookup()
            const {
        return dynamic_col_idx_lookup_;
    }

    inline size_t get_num_dynamic_columns() const {
        return dynamic_col_idx_lookup_.size();
    }

    inline bool get_dynamic_array_flag(size_t col_idx) const {
        return columns_.at(col_idx).get_dynamic_array_flag();
    }

private:
    void append_column(const Column &column) {
        size_t col_idx = columns_.size();
        columns_.emplace_back(column);

        size_t old_row_size = *offsets_.rbegin();
        size_t new_row_size = old_row_size + columns_.rbegin()->get_data_size();
        offsets_.push_back(new_row_size);

        if (column.get_dynamic_array_flag()) {
            size_t num_dynamic_cols_sofar = dynamic_col_idx_lookup_.size();
            dynamic_col_idx_lookup_[col_idx] = num_dynamic_cols_sofar;
        }
    }


    inline void append_column(const std::string &name, const Data_Type &type,
                              const size_t &array_size,
                              const Field_Properties &field_properties,
                              bool dynamic_array_flag) {
        append_column(
                Column(name, type, array_size, field_properties, dynamic_array_flag));
    }

    std::vector<Column> columns_;
    std::vector<size_t> offsets_;

    std::unordered_map<size_t, size_t> dynamic_col_idx_lookup_;
};

}  // namespace tablator
