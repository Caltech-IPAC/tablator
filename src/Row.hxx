#pragma once

#include <cassert>
#include <vector>

#include <boost/spirit/include/qi.hpp>

#include "Common.hxx"
#include "Data_Type.hxx"

//#include <iostream>

namespace tablator {

class Field_Framework;
class Table;

class Row {
public:
    Row(const size_t &data_size, const size_t &num_dynamic_columns = 0)
            : data_(data_size), dynamic_array_sizes_(num_dynamic_columns) {}

    Row(const Table &table);

    Row(const Field_Framework &field_framework);

    void fill_with_zeros() {
        std::fill(data_.begin(), data_.end(), 0);
        std::fill(dynamic_array_sizes_.begin(), dynamic_array_sizes_.end(), 0);
    }

    void insert_null(
            Data_Type type, const size_t &array_size, const size_t &col_idx,
            const size_t &offset, const size_t &offset_end,
            const size_t &idx_in_dynamic_cols_list = DEFAULT_IDX_IN_DYNAMIC_COLS_LIST);

    template <typename T>
    void insert(
            const T &element, const size_t &offset,
            const size_t &idx_in_dynamic_cols_list = DEFAULT_IDX_IN_DYNAMIC_COLS_LIST) {

        assert(offset + sizeof(T) <= data_.size());
        std::copy(reinterpret_cast<const char *>(&element),
                  reinterpret_cast<const char *>(&element) + sizeof(T),
                  data_.data() + offset);
        increment_array_size_if_dynamic(idx_in_dynamic_cols_list);
    }

    template <typename T>
    void insert(
            const T &begin, const T &end, const size_t &offset,
            const size_t &idx_in_dynamic_cols_list = DEFAULT_IDX_IN_DYNAMIC_COLS_LIST) {
        assert(offset + std::distance(begin, end) <= data_.size());
        std::copy(begin, end, data_.data() + offset);

        set_array_size_if_dynamic(idx_in_dynamic_cols_list,
                                  std::distance(begin, end) / sizeof(T));
    }

    // Called by add_cntr_column()
    void insert_data_only(const uint8_t *begin, const uint8_t *end,
                          const size_t &offset) {
        assert(offset + std::distance(begin, end) <= data_.size());
        std::copy(reinterpret_cast<const char *>(begin),
                  reinterpret_cast<const char *>(end), data_.data() + offset);
    }

    // Called by add_cntr_column().
    void set_dynamic_array_sizes(const std::vector<uint32_t> &new_sizes) {
        dynamic_array_sizes_.insert(dynamic_array_sizes_.end(), new_sizes.begin(),
                                    new_sizes.end());
    }


    // called by insert_from_ascii()
    void insert(
            const std::string &element, const size_t &offset_begin,
            const size_t &offset_end,
            const size_t &idx_in_dynamic_cols_list = DEFAULT_IDX_IN_DYNAMIC_COLS_LIST) {
        std::string element_copy(element);
        element_copy.resize(offset_end - offset_begin, '\0');
        std::copy(element_copy.begin(), element_copy.end(),
                  data_.data() + offset_begin);

        set_array_size_if_dynamic(
                idx_in_dynamic_cols_list,
                std::distance(element_copy.begin(), element_copy.end()));
    }

    void insert_from_ascii(const std::string &value, const Data_Type &data_type,
                           const size_t &array_size, const size_t &col_idx,
                           const size_t &offset, const size_t &offset_end,
                           const size_t &idx_in_dynamic_cols_list);


    void insert_from_bigendian(const std::vector<uint8_t> &stream,
                               size_t starting_src_pos, const Data_Type &data_type,
                               const size_t &array_size, const size_t &offset,
                               const size_t &idx_in_dynamic_cols_list);

    size_t get_size() const { return data_.size(); }

    const std::vector<char> &get_data() const { return data_; }
    std::vector<char> &get_data() { return data_; }


    const std::vector<uint32_t> &get_dynamic_array_sizes() const {
        return dynamic_array_sizes_;
    }
    std::vector<uint32_t> &get_dynamic_array_sizes() { return dynamic_array_sizes_; }


    inline void set_dynamic_array_size(const size_t &dyn_col_idx,
                                       const size_t &dyn_size) {
        if (dynamic_array_sizes_.size() < dyn_col_idx + 1) {
            throw std::runtime_error(
                    "set_dynamic_array_size(): dynamic_array_sizes vector improperly "
                    "initialized");
        }
        dynamic_array_sizes_.at(dyn_col_idx) = dyn_size;
    }

    inline void increment_dynamic_array_size(const size_t &dyn_col_idx) {
        if (dynamic_array_sizes_.size() < dyn_col_idx + 1) {
            throw std::runtime_error(
                    "increment_dynamic_array_size(): dynamic_array_sizes vector "
                    "improperly initialized");
        }
        dynamic_array_sizes_.at(dyn_col_idx) = dynamic_array_sizes_.at(dyn_col_idx) + 1;
    }


    void set_array_size_if_dynamic(const size_t &dyn_col_idx,
                                   const size_t &array_size) {
        if (dyn_col_idx != DEFAULT_IDX_IN_DYNAMIC_COLS_LIST) {
            set_dynamic_array_size(dyn_col_idx, array_size);
        }
    }

    void increment_array_size_if_dynamic(const size_t &dyn_col_idx) {
        if (dyn_col_idx != DEFAULT_IDX_IN_DYNAMIC_COLS_LIST) {
            increment_dynamic_array_size(dyn_col_idx);
        }
    }

private:
    template <class T, class Rule>
    void insert_from_bigendian_internal(size_t column_offset, const Rule &rule,
                                        size_t array_size,
                                        const std::vector<uint8_t> &stream,
                                        size_t starting_src_pos);

    template <typename T>
    void insert_null_internal(const size_t &offset) {
        insert(tablator::get_null<T>(), offset);
    }

    void insert_null_by_type(Data_Type data_type, const size_t &offset);


    // JTODO Make this vector<uint8_t>, like Data_Details.data_.
    std::vector<char> data_;
    std::vector<uint32_t> dynamic_array_sizes_;
};


}  // namespace tablator
