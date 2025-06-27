#pragma once

#include <set>

#include "../Column.hxx"
#include "../Field_Framework.hxx"
#include "../Row.hxx"

namespace tablator {

// row-related functions
inline void append_row(std::vector<uint8_t> &data, const Row &row) {
    data.insert(data.end(), row.get_data().begin(), row.get_data().end());
}

inline void append_rows(std::vector<uint8_t> &data, const std::vector<uint8_t> &data2) {
    data.insert(data.end(), data2.begin(), data2.end());
}

void winnow_rows(std::vector<uint8_t> &data,
				 const std::set<size_t> &selected_row_idx_list,
				 size_t num_rows, size_t row_size);

}  // namespace tablator
