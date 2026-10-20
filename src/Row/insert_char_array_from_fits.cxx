#include <algorithm>
#include <cstring>
#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../Row.hxx"
#include "../data_size.hxx"

namespace tablator {

// Caller has handled nulls
void Row::insert_char_array_from_fits(std::vector<std::vector<char>> &data_vec,
                                      const size_t &max_array_size,
                                      const size_t &offset, const size_t &col_idx,
                                      const size_t &substring_size,
                                      const size_t &num_substrings,
                                      bool dynamic_array_flag) {
    // A cell's substrings are stored contiguously. Substring i starts at
    // offset + i * substring_size, where offset is the start of the cell.
    if (num_substrings * substring_size > max_array_size) {
        throw std::runtime_error("Substrings don't fit in the cell's array.");
    }

    // Each substring before the cell's last non-empty one is padded
    // with spaces to substring_size, matching its layout in the FITS
    // file.  Empty substrings after the last non-empty one are
    // dropped.
    size_t num_used_substrings = 0;
    for (size_t i = 0; i < num_substrings; ++i) {
        if (data_vec[i][0] != '\0') {
            num_used_substrings = i + 1;
        }
    }

    size_t offset_begin = offset;
    uint32_t curr_array_size = 0;
    for (size_t i = 0; i < num_used_substrings; ++i) {
        const char *element = data_vec[i].data();
        size_t elt_length = std::strlen(element);
        if (elt_length > substring_size) {
            throw std::runtime_error("Array element is too long, idx: " +
                                     std::to_string(i));
        }
        char *dest = data_.data() + offset_begin;
        std::copy(element, element + elt_length, dest);
        if (i + 1 < num_used_substrings) {
            std::fill(dest + elt_length, dest + substring_size, ' ');
            curr_array_size += substring_size;
        } else {
            curr_array_size += elt_length;
        }
        offset_begin += substring_size;
    }

    if (dynamic_array_flag) {
        // JTODO tablator's variable-array size is not the same as FITS's.
        set_dynamic_array_size(col_idx, curr_array_size);
    }
}

}  // namespace tablator
