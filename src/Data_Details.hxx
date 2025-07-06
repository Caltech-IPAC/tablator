#pragma once

#include "Common.hxx"
#include "Field_Framework.hxx"
#include "Row.hxx"

namespace tablator {

class Data_Details {
public:
    Data_Details(size_t row_size, size_t num_rows = 0) : row_size_(row_size) {
        init(num_rows);
    }

    Data_Details(const Field_Framework &field_framework, size_t num_rows = 0)
            : Data_Details(field_framework.get_row_size(), num_rows) {}


    void append_row(const Row &row) {
        assert(row.get_data().size() == get_row_size());
        data_.reserve(data_.size() + row.get_data().size());
        data_.insert(data_.end(), row.get_data().begin(), row.get_data().end());
    }


    void append_rows(const Data_Details &other) {
        assert(other.get_row_size() == get_row_size());

        data_.reserve(data_.size() + other.get_data().size());
        data_.insert(data_.end(), other.get_data().begin(), other.get_data().end());
    }

    void adjust_num_rows(const size_t new_num_rows) {
        data_.resize(new_num_rows * get_row_size());
    }

    void reserve_rows(const size_t &new_num_rows) {
        data_.reserve(get_row_size() * new_num_rows);
    }

    // accessors

    size_t get_data_size() const { return data_.size(); }

    size_t get_num_rows() const {
        if (get_row_size() == 0) {
            return 0;
        }
        return get_data_size() / get_row_size();
    };

    inline size_t get_row_size() const { return row_size_; }

    inline const std::vector<uint8_t> &get_data() const { return data_; }

    // Non-const to support append_rows().
    inline std::vector<uint8_t> &get_data() { return data_; }

    inline void set_data(const std::vector<uint8_t> &d) { data_ = d; }


private:
    void init(const size_t &new_num_rows) {
	  reserve_rows(new_num_rows);
    }

    // Can't be const because of append_rows().
    std::vector<uint8_t> data_;
    size_t row_size_;
};

}  // namespace tablator
