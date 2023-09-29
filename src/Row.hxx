#pragma once

#include <cassert>
#include <vector>

#include "Data_Type.hxx"
#include "Utils/Data_Array.hxx"
#include "Utils/Null_Utils.hxx"

namespace tablator {
class Row {
public:
    std::vector<char> data_;

    Row(const size_t &size) : data_(size) {}

    void fill_with_zeros() { std::fill(data_.begin(), data_.end(), 0); }

    // backward compatibility
    void set_zero() { fill_with_zeros(); }

    void set_null(const Data_Type &type, const size_t &array_size,
                  const size_t &col_idx, size_t offset, size_t offset_end) {
        tablator::set_null(data_, 0, type, array_size, col_idx, offset, offset_end);
    }

    template <typename T>
    void insert(const T &element, const size_t &offset) {
        tablator::insert(data_, 0, element, offset);
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
        std::copy(element_copy.begin(), element_copy.end(),
                  data_.data() + offset_begin);
    }


    template <typename T>
    void insert_null(const size_t &offset) {
        tablator::insert_null<T>(data_, 0, offset);
    }

    size_t size() const { return data_.size(); }

private:
    void set_null_internal(const Data_Type &data_type, const size_t &offset) {
        tablator::set_null_internal(data_, 0, data_type, offset);
    }
};
}  // namespace tablator
