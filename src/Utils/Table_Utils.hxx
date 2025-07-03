#pragma once

#include <set>

#include "../Column.hxx"
#include "../Field_Framework.hxx"
#include "../Row.hxx"

namespace tablator {

// JTODO Move to Data_Details
void retain_only_selected_rows(std::vector<uint8_t> &data,
                               const std::set<size_t> &selected_row_idx_list,
                               size_t num_rows, uint row_size);


}  // namespace tablator
