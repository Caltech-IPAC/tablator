#pragma once

#include <cassert>
#include <vector>

#include "Common.hxx"
#include "Data_Type.hxx"

#include <iostream>

namespace tablator {
class Row {
public:


    Row(size_t data_size, size_t num_dynamic_arrays = 0)
            : data_(data_size), dynamic_array_sizes_(num_dynamic_arrays, 0) {}

    void fill_with_zeros() {
        std::fill(data_.begin(), data_.end(), 0);
        std::fill(dynamic_array_sizes_.begin(), dynamic_array_sizes_.end(), 0);
    }

    void insert_null(
            Data_Type type, size_t array_size, size_t col_idx, size_t offset,
            size_t offset_end,
            size_t index_in_dynamic_cols_list = DEFAULT_INDEX_IN_DYNAMIC_COLS_LIST);

    template <typename T>
    void insert(
            const T &element, size_t offset,
            size_t index_in_dynamic_cols_list = DEFAULT_INDEX_IN_DYNAMIC_COLS_LIST) {
        assert(offset + sizeof(T) <= data_.size());

        // JTODO this is inappropriate if we're inserting array elements one by one.
        if (index_in_dynamic_cols_list != DEFAULT_INDEX_IN_DYNAMIC_COLS_LIST) {
            increment_dynamic_array_size(index_in_dynamic_cols_list);
        }
        std::copy(reinterpret_cast<const char *>(&element),
                  reinterpret_cast<const char *>(&element) + sizeof(T),
                  data_.data() + offset);
    }

    template <typename T>
    void insert(
            const T &begin, const T &end, size_t offset,
            size_t index_in_dynamic_cols_list = DEFAULT_INDEX_IN_DYNAMIC_COLS_LIST) {
        assert(offset + std::distance(begin, end) <= data_.size());

        if (index_in_dynamic_cols_list != DEFAULT_INDEX_IN_DYNAMIC_COLS_LIST) {
            set_dynamic_array_size(index_in_dynamic_cols_list,
                                   std::distance(begin, end) / sizeof(T));  // JTODO
        }

        std::copy(begin, end, data_.data() + offset);
    }


    void insert(
            const std::string &element, size_t offset_begin, size_t offset_end,
            size_t index_in_dynamic_cols_list = DEFAULT_INDEX_IN_DYNAMIC_COLS_LIST) {
        std::string element_copy(element);
        element_copy.resize(offset_end - offset_begin, '\0');

        if (index_in_dynamic_cols_list != DEFAULT_INDEX_IN_DYNAMIC_COLS_LIST) {
            set_dynamic_array_size(
                    index_in_dynamic_cols_list,
                    std::distance(element_copy.begin(), element_copy.end()));
        }

        std::copy(element_copy.begin(), element_copy.end(),
                  data_.data() + offset_begin);
    }


    void insert_from_ascii(const std::string &element, const Data_Type &data_type,
                           const size_t &array_size, const size_t &column,
                           const size_t &offset, const size_t &offset_end);


    size_t get_size() const { return data_.size(); }

    const std::vector<char> &get_data() const { return data_; }
    std::vector<char> &get_data() { return data_; }

    const std::vector<uint32_t> &get_dynamic_array_sizes() const {
        return dynamic_array_sizes_;
    }
    std::vector<uint32_t> &get_dynamic_array_sizes() { return dynamic_array_sizes_; }

    inline void set_dynamic_array_size(size_t dyn_col_idx, size_t dyn_size) {
        if (dynamic_array_sizes_.size() < dyn_col_idx + 1) {
            throw std::runtime_error(
                    "dynamic_array_sizes vector improperly initialized");
        }
        dynamic_array_sizes_.at(dyn_col_idx) = dyn_size;
    }

    inline void increment_dynamic_array_size(size_t dyn_col_idx) {
        if (dynamic_array_sizes_.size() < dyn_col_idx + 1) {
            throw std::runtime_error(
                    "dynamic_array_sizes vector improperly initialized");
        }
        dynamic_array_sizes_.at(dyn_col_idx) = dynamic_array_sizes_.at(dyn_col_idx) + 1;
    }

private:
    template <typename T>
    void insert_null_internal(size_t offset) {
        insert(tablator::get_null<T>(), offset);
    }

    void insert_null_by_type(Data_Type data_type, size_t offset);

    std::vector<char> data_;
    std::vector<uint32_t> dynamic_array_sizes_;
};


}  // namespace tablator
