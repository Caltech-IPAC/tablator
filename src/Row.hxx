#pragma once

#include <cassert>
#include <vector>

#include "Data_Type.hxx"
#include "unsafe_copy_to_row.hxx"

namespace tablator {
class Row {
public:
    // JTODO Update tablator clients to call get_data() and then make data a private
    // class member.
  
  // JTODO Reconcile char vs. uint8_t.

  //    std::vector<char> data;

    Row(const size_t &size) : data_(size) {}

    void fill_with_zeros() { std::fill(data_.begin(), data_.end(), 0); }

    // backward compatibility
    void set_zero() { fill_with_zeros(); }

    size_t set_null(const Data_Type &type, const size_t &array_size,
                  const size_t &col_idx, const size_t &offset,
                  const size_t &offset_end, bool dynamic_array_flag);

    template <typename T>
    void insert(const T &element, const size_t &offset) {
        assert(offset + sizeof(T) <= data_.size());
        unsafe_copy_to_row(element, offset, data_.data());
    }

    template <typename T>
    void insert(const T &begin, const T &end, const size_t &offset) {
        assert(offset + std::distance(begin, end) <= data_.size());
        std::copy(begin, end, data_.data() + offset);
    }

    void insert(const std::string &element, const size_t &offset_begin,
                const size_t &offset_end) {
        std::string element_copy(element);
        element_copy.resize(offset_end - offset_begin, '\0');
        std::copy(element_copy.begin(), element_copy.end(), data_.data() + offset_begin);
    }

    template <typename T>
    void insert_null(const size_t &offset) {
        insert(tablator::get_null<T>(), offset);
    }

    size_t get_size() const { return data_.size(); }
    // Deprecated
    size_t size() const { return get_size(); }

    const std::vector<char> &get_data() const { return data_; }
    std::vector<char> &get_data() { return data_; }

private:
    void set_null_internal(const Data_Type &data_type, const size_t &offset);
    std::vector<char> data_;
};


}  // namespace tablator
