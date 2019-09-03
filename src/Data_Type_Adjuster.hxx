#pragma once
#include "Table.hxx"

namespace tablator {

class Data_Type_Adjuster {
public:
    Data_Type_Adjuster(const Table &table) : table_(table) {}

    std::vector<tablator::Data_Type> get_datatypes_for_writing(
            const Format::Enums &enum_format) const;

    static size_t get_char_array_size_for_uint64_col(size_t uint64_array_size) {
        // allow for spaces in between elements
        return uint64_array_size * (std::numeric_limits<uint64_t>::digits10 + 1) - 1;
    }

private:
    bool contains_large_uint64_val(size_t col) const;

    const Table &table_;
};
}  // namespace tablator
