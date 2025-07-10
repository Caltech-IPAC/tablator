#pragma once

#include <cstddef>
#include <cstdint>
#include <set>
#include <vector>

namespace tablator {

// JTODO Move to Data_Details
void winnow_rows(std::vector<uint8_t> &data,
                 const std::set<size_t> &selected_row_idx_list, size_t num_rows,
                 size_t row_size);

}  // namespace tablator
