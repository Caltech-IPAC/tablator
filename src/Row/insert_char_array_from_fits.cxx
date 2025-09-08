#include <stdexcept>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../Row.hxx"
#include "../data_size.hxx"

namespace tablator {

// Caller has handled nulls
void Row::insert_char_array_from_fits(std::vector<std::vector<char>> &data_vec,
                                      const size_t &max_array_size,
                                      const size_t &offset, const size_t &offset_end,
                                      const size_t &col_idx,
                                      const size_t &substring_size,
                                      const size_t &num_substrings,
                                      bool dynamic_array_flag) {
    size_t offset_begin = offset;
    uint32_t curr_array_size = 0;
    for (size_t i = 0; i < num_substrings; ++i) {
        char *element = data_vec[i].data();
        size_t elt_length = strlen(element);
        std::copy(element, element + elt_length, data_.data() + offset_begin);
        curr_array_size += (i == num_substrings - 1) ? elt_length : substring_size;

        offset_begin += max_array_size;
    }

    if (dynamic_array_flag) {
        // JTODO tablator's variable-array size is not the same as FITS's.
        set_dynamic_array_size(col_idx, curr_array_size);
    }
}

}  // namespace tablator
