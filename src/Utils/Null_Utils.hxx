#pragma once

#include <iostream>

namespace tablator {

static const std::string null_bitfield_flags_name("null_bitfield_flags");
static const std::string null_bitfield_flags_description(
        "Packed bit array indicating whether an entry is null");

// Computes the minimum number of bytes which collectively contain
// n distinct bits.
inline size_t bits_to_bytes(size_t n) { return (n + 7) / 8; }


  // Following the VOTable convention, we use the most significant
  // bit for the first column.
inline bool is_null_MSB(const std::vector<uint8_t> &data, const size_t &row_offset,
                        const size_t &col_idx) {
    return data[row_offset + (col_idx - 1) / 8] & (128 >> ((col_idx - 1) % 8));
}

}  // namespace tablator
