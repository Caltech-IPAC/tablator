#pragma once

#include <cassert>
#include <vector>

#include <boost/spirit/include/qi.hpp>

#include "Common.hxx"
#include "Data_Type.hxx"

namespace tablator {
class Row {
public:
    Row(const size_t &data_size) : data_(data_size) {}

    void fill_with_zeros() { std::fill(data_.begin(), data_.end(), 0); }

    void insert_null(Data_Type type, const size_t &array_size, const size_t &col_idx,
                     const size_t &offset, const size_t &offset_end);

    template <typename T>
    void insert(const T &element, const size_t &offset) {
        assert(offset + sizeof(T) <= data_.size());
        std::copy(reinterpret_cast<const char *>(&element),
                  reinterpret_cast<const char *>(&element) + sizeof(T),
                  data_.data() + offset);
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


    void insert_from_ascii(const std::string &value, const Data_Type &data_type,
                           const size_t &array_size, const size_t &col_idx,
                           const size_t &offset, const size_t &offset_end);


    void insert_from_bigendian(const std::vector<uint8_t> &stream,
                               size_t starting_src_pos, const Data_Type &data_type,
                               const size_t &array_size,
                               const size_t &offset);


    size_t get_size() const { return data_.size(); }

    const std::vector<char> &get_data() const { return data_; }
    std::vector<char> &get_data() { return data_; }

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

    std::vector<char> data_;
};


}  // namespace tablator
