#pragma once

#include <set>
#include <unordered_map>

#include "Common.hxx"
#include "Row.hxx"

namespace tablator {

class Field_Framework;

class Data_Details {
public:
    Data_Details(const std::unordered_map<size_t, size_t> &dynamic_col_idx_lookup,
                 size_t row_size, size_t num_rows = 0)
            : dynamic_col_idx_lookup_(dynamic_col_idx_lookup),
              num_dynamic_columns_(dynamic_col_idx_lookup.size()),
              row_size_(row_size) {
        init(num_rows);
    }

    Data_Details(const Field_Framework &field_framework, size_t num_rows = 0);


    void append_row(const Row &row);

    // This function is called only by Table::append_rows(), which
    // checks that the Field_Frameworks of the two tables are
    // compatible.
    void append_rows(const Data_Details &other);

    void add_cntr_column(const Field_Framework &dest_ff, const Field_Framework &src_ff,
                         const Data_Details &src_dd);

    void combine_data_details(const Field_Framework &dest_ff,
                              const Field_Framework &src1_ff,
                              const Field_Framework &src2_ff,
                              const Data_Details &src1_dd, const Data_Details &src2_dd);

    void winnow_rows(const std::set<size_t> &selected_row_idx_list);

    inline void adjust_num_rows(size_t new_num_rows) {
        data_.resize(new_num_rows * get_row_size());
        dynamic_array_sizes_by_row_.resize(new_num_rows);
    }

    inline void reserve_rows(const size_t &new_num_rows) {
        data_.reserve(get_row_size() * new_num_rows);
    }

    // accessors

    inline const std::vector<uint8_t> &get_data() const { return data_; }

    // Non-const to support append_rows().
    inline std::vector<uint8_t> &get_data() { return data_; }

    inline void set_data(const std::vector<uint8_t> &d) { data_ = d; }

    inline size_t get_num_dynamic_columns() const { return num_dynamic_columns_; }

    inline bool got_dynamic_columns() const { return num_dynamic_columns_ > 0; }

    inline size_t get_data_size() const { return data_.size(); }

    inline size_t get_num_rows() const { return get_data_size() / get_row_size(); };

    inline size_t get_row_size() const { return row_size_; }

    inline const std::unordered_map<size_t, size_t> &get_dynamic_col_idx_lookup()
            const {
        return dynamic_col_idx_lookup_;
    }

    inline const std::vector<std::vector<uint32_t>> &get_dynamic_array_sizes_by_row()
            const {
        return dynamic_array_sizes_by_row_;
    }

    inline const std::vector<uint32_t> &get_dynamic_array_sizes(size_t row_idx) const {
        return get_dynamic_array_sizes_by_row().at(row_idx);
    }

private:
    void init(const size_t &new_num_rows) {
        reserve_rows(new_num_rows);
        if (get_num_dynamic_columns()) {
            dynamic_array_sizes_by_row_.reserve(new_num_rows);

            for (uint i = 0; i < new_num_rows; ++i) {
                dynamic_array_sizes_by_row_.emplace_back();
                dynamic_array_sizes_by_row_.back().reserve(get_num_dynamic_columns());
            }
        }
    }

    // Can't be const because of append_rows().
    // JTODO data_ could also be made 2-dim'l.
    std::vector<uint8_t> data_;
    std::unordered_map<size_t, size_t> dynamic_col_idx_lookup_;
    std::vector<std::vector<uint32_t>> dynamic_array_sizes_by_row_;

    size_t num_dynamic_columns_;
    size_t row_size_;
};

}  // namespace tablator
