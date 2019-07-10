#pragma once
#include "Table.hxx"

namespace tablator {

class Data_Type_Adjuster {
public:
    Data_Type_Adjuster(const Table &table) : table_(table) {}

    std::vector<tablator::Data_Type> get_datatypes_for_writing(
            const Format::Enums &enum_format) const;

    static Data_Type get_datatype_for_writing(
            const Table &table, const std::vector<Data_Type> &datatypes_for_writing,
            size_t col);

    static size_t get_char_array_size_for_uint64_col(size_t uint64_array_size) {
        // allow for spaces in between elements
        return uint64_array_size * (std::numeric_limits<uint64_t>::digits10 + 1) - 1;
    }

private:
    bool contains_large_uint64_val(size_t col) const;

    static bool sanity_check(Data_Type old_datatype, Data_Type new_datatype) {
        static std::multimap<Data_Type, Data_Type> legal_adjustments = {
                {Data_Type::UINT64_LE, Data_Type::CHAR},
                {Data_Type::UINT64_LE, Data_Type::INT64_LE},
                {Data_Type::UINT32_LE, Data_Type::INT64_LE},
                {Data_Type::UINT16_LE, Data_Type::INT32_LE},
        };
        if (old_datatype == new_datatype) {
            return true;
        }
        auto lower = legal_adjustments.lower_bound(old_datatype);
        if (lower == legal_adjustments.end() || lower->first != old_datatype) {
            return false;
        }
        auto upper = legal_adjustments.upper_bound(old_datatype);
        for (auto iter = lower; iter != upper; ++iter) {
            if (iter->second == new_datatype) {
                return true;
            }
        }
        return false;
    }

    const Table &table_;
};
}  // namespace tablator
