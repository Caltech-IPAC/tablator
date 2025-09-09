#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>

#include <boost/spirit/include/qi.hpp>

#include "Common.hxx"
#include "Data_Type.hxx"

namespace tablator {

class Field_Framework;
class Table;

class Row {
public:
    Row(const Table &table);

    Row(const Field_Framework &field_framework);

    void fill_with_zeros() {
        std::fill(data_.begin(), data_.end(), 0);
        std::fill(dynamic_array_sizes_.begin(), dynamic_array_sizes_.end(), 0);
    }

    void insert_null(Data_Type type, const size_t &array_size, const size_t &offset,
                     const size_t &offset_end, const size_t &col_idx,
                     bool dynamic_array_flag);

    template <typename T>
    void insert(const T &element, const size_t &offset, const size_t &col_idx,
                bool dynamic_array_flag) {
        assert(offset + sizeof(T) <= data_.size());
        std::copy(reinterpret_cast<const char *>(&element),
                  reinterpret_cast<const char *>(&element) + sizeof(T),
                  data_.data() + offset);

        if (dynamic_array_flag) {
            increment_dynamic_array_size(col_idx);
        }
    }

    template <typename T>
    void insert(const T &begin, const T &end, const size_t &offset,
                const size_t &col_idx, bool dynamic_array_flag) {
        assert(offset + std::distance(begin, end) <= data_.size());
        std::copy(begin, end, data_.data() + offset);
        if (dynamic_array_flag) {
            set_dynamic_array_size(
                    col_idx, std::distance(begin, end));  // distance between pointers
        }
    }

    template <typename T>
    void insert(const T &element, const size_t &offset) {
        insert(element, offset, 1 /* random col_idx */, false /* dynamic_array_flag */);
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
    void insert(const std::string &element, const size_t &offset_begin,
                const size_t &offset_end, const size_t &col_idx,
                bool dynamic_array_flag) {
        std::string element_copy(element);
        element_copy.resize(offset_end - offset_begin, '\0');
        std::copy(element_copy.begin(), element_copy.end(),
                  data_.data() + offset_begin);

        if (dynamic_array_flag) {
            set_dynamic_array_size(
                    col_idx, std::distance(element_copy.begin(), element_copy.end()));
        }
    }

    void insert_char_array_from_fits(std::vector<std::vector<char>> &data_vec,
                                     const size_t &max_array_size, const size_t &offset,
                                     const size_t &col_idx,
                                     const size_t &substring_size,
                                     const size_t &num_substrings,
                                     bool dynamic_array_flag);

    void insert_from_ascii(const std::string &value, const Data_Type &data_type,
                           const size_t &max_array_size, const size_t &offset,
                           const size_t &offset_end, const size_t &col_idx,
                           const size_t &curr_array_size, bool dynamic_array_flag);


    void insert_from_bigendian(const std::vector<uint8_t> &stream,
                               size_t starting_src_pos, const Data_Type &data_type,
                               const size_t &array_size, const size_t &offset,
                               const size_t &col_idx, bool dynamic_array_flag);


    size_t get_size() const { return data_.size(); }

    const std::vector<char> &get_data() const { return data_; }
    std::vector<char> &get_data() { return data_; }

    inline const std::unordered_map<size_t, size_t> &get_dynamic_col_idx_lookup()
            const {
        return dynamic_col_idx_lookup_;
    }

    const std::vector<uint32_t> &get_dynamic_array_sizes() const {
        return dynamic_array_sizes_;
    }

    std::vector<uint32_t> &get_dynamic_array_sizes() { return dynamic_array_sizes_; }

    inline void set_dynamic_array_size(const size_t &col_idx, const size_t &dyn_size) {
        const auto iter = dynamic_col_idx_lookup_.find(col_idx);
        if (iter == dynamic_col_idx_lookup_.end()) {
            throw std::runtime_error(
                    "set_dynamic_array_size(): no lookup entry for col_idx");
        }
        dynamic_array_sizes_.at(iter->second) = dyn_size;
    }


    inline void increment_dynamic_array_size(const size_t &dyn_col_idx) {
        const auto iter = dynamic_col_idx_lookup_.find(dyn_col_idx);
        if (iter == dynamic_col_idx_lookup_.end()) {
            throw std::runtime_error(
                    "increment_dynamic_array_size(): no lookup entry for col_idx");
        }
        dynamic_array_sizes_.at(iter->second) =
                dynamic_array_sizes_.at(iter->second) + 1;
    }

private:
    template <class T>
    void insert_from_bigendian_internal(size_t column_offset, size_t array_size,
                                        const std::vector<uint8_t> &stream,
                                        size_t starting_src_pos);

    template <typename T>
    void insert_null_internal(const size_t &offset) {
        insert(tablator::get_null<T>(), offset);
    }

    void insert_null_by_type(Data_Type data_type, const size_t &offset);


    // JTODO Make this vector<uint8_t>, like Data_Details.data_.
    std::vector<char> data_;
    const std::unordered_map<size_t, size_t> &dynamic_col_idx_lookup_;
    std::vector<uint32_t> dynamic_array_sizes_;
};


}  // namespace tablator
