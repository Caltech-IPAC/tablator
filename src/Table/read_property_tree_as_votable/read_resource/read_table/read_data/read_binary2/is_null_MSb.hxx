#pragma once

#include <cstdint>
#include <cstdlib>
#include <vector>

namespace {
inline bool is_null_MSb(const std::vector<uint8_t> &data, const size_t &row_offset,
                        const size_t &column) {
    return data[row_offset + (column - 1) / 8] & (128 >> ((column - 1) % 8));
}
}  // namespace
