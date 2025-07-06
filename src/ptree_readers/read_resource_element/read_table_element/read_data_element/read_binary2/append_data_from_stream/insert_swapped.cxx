#include "../../../../../../ptree_readers.hxx"

#include <boost/spirit/include/qi.hpp>

#include "../../../../../../Utils/Table_Utils.hxx"

namespace {

template <class T, class Rule>
T swap_big_ended_array_element_element(const Rule &rule,
                                       std::vector<uint8_t>::const_iterator &src_begin,
                                       std::vector<uint8_t>::const_iterator &src_end) {
    T result = 0;
    boost::spirit::qi::parse(src_begin, src_end, rule, result);
    return result;
}

//=================================================================

void insert_swapped_array_element(uint8_t *temp_ptr,
                                  std::vector<uint8_t>::const_iterator &src_begin,
                                  std::vector<uint8_t>::const_iterator &src_end,
                                  size_t data_type_size, const bool &is_bool) {
    switch (data_type_size) {
        case 1:
            // boolean serialization is a little different from just using 0
            // or 1.  Instead, it uses a serialization of the character
            // representation of true or false
            if (is_bool) {
                if (*src_begin != 't' && *src_begin != 'T' && *src_begin != '1' &&
                    *src_begin != 'f' && *src_begin != 'F' && *src_begin != '0')
                    throw std::runtime_error(
                            "Invalid value for boolean.  The "
                            "integer representation is " +
                            std::to_string(*src_begin));
                *temp_ptr =
                        ((*src_begin == 't' || *src_begin == 'T' || *src_begin == '1')
                                 ? static_cast<uint8_t>(1)
                                 : static_cast<uint8_t>(0));
            } else {
                *temp_ptr = *src_begin;
            }
            break;
        case 2:
            *(reinterpret_cast<uint16_t *>(temp_ptr)) =
                    swap_big_ended_array_element_element<uint16_t>(
                            boost::spirit::qi::big_word, src_begin, src_end);
            break;
        case 4:
            *(reinterpret_cast<uint32_t *>(temp_ptr)) =
                    swap_big_ended_array_element_element<uint32_t>(
                            boost::spirit::qi::big_word, src_begin, src_end);
            break;
        case 8:
            *(reinterpret_cast<uint64_t *>(temp_ptr)) =
                    swap_big_ended_array_element_element<uint64_t>(
                            boost::spirit::qi::big_word, src_begin, src_end);
            break;
        default:
            throw std::runtime_error("Invalid value for data_type_size: " +
                                     std::to_string(data_type_size));
            break;
    }
}

//=================================================================

void insert_swapped_array(tablator::Row &row, size_t column_offset,
                          const std::vector<uint8_t> temp_vec, size_t data_type_size,
                          size_t possibly_dynamic_array_size) {
    switch (data_type_size) {
        case 1: {
            row.insert(temp_vec.data(), temp_vec.data() + possibly_dynamic_array_size,
                       column_offset);
        } break;

        case 2: {
            const std::vector<uint16_t> temp_type_vec(temp_vec.begin(), temp_vec.end());
            row.insert(temp_type_vec.data(),
                       temp_type_vec.data() + possibly_dynamic_array_size,
                       column_offset);
        } break;
        case 4: {
            const std::vector<uint16_t> temp_type_vec(temp_vec.begin(), temp_vec.end());
            row.insert(temp_type_vec.data(),
                       temp_type_vec.data() + possibly_dynamic_array_size,
                       column_offset);
        } break;
        case 8: {
            const std::vector<uint16_t> temp_type_vec(temp_vec.begin(), temp_vec.end());
            row.insert(temp_type_vec.data(),
                       temp_type_vec.data() + possibly_dynamic_array_size,
                       column_offset);
        } break;
        default: {
            throw std::runtime_error("Invalid value for data_type_size: " +
                                     std::to_string(data_type_size));
        } break;
    }
}

}  // namespace

//========================================================

namespace tablator {
void insert_swapped(Row &row, size_t column_offset, const Data_Type &data_type,
                    size_t possibly_dynamic_array_size,
                    const std::vector<uint8_t> &stream, size_t starting_src_pos) {
    const size_t data_type_size(data_size(data_type));
    const bool is_bool(data_type == Data_Type::INT8_LE);
    size_t src_pos = starting_src_pos;

    std::vector<uint8_t> temp_vec(possibly_dynamic_array_size * data_type_size);

    // Insert array elements into temp_vec, swapping endedness, and
    // then insert as a group into row.
    uint8_t *curr_temp_ptr = temp_vec.data();

    for (size_t index = 0; index != possibly_dynamic_array_size; ++index) {
        auto src_begin = stream.begin();
        std::advance(src_begin, src_pos);
        auto src_end = stream.begin();
        src_pos += data_type_size;
        std::advance(src_end, src_pos);

        insert_swapped_array_element(curr_temp_ptr, src_begin, src_end, data_type_size,
                                     is_bool);
        curr_temp_ptr += data_type_size;
    }
    insert_swapped_array(row, column_offset, temp_vec, data_type_size,
                         possibly_dynamic_array_size);
}
}  // namespace tablator
