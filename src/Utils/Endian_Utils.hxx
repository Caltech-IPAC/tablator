#pragma once

#include <cstdint>

namespace tablator {

//============================================================
// Convert from  little-endian to big-endian or vice versa
//============================================================

// Copy data_size bytes in reverse order from sec to dest.

inline void copy_swapped_bytes(uint8_t *dest, const uint8_t *src, size_t data_size) {
    for (size_t i = 0; i < data_size; i++) {
        dest[data_size - 1 - i] = src[i];
    }
}

}  // namespace tablator
