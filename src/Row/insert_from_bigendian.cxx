#include "../Row.hxx"

#include <stdexcept>

#include "../Utils/Endian_Utils.hxx"
#include "../data_size.hxx"

namespace tablator {

template <class T>
void Row::insert_from_bigendian_internal(size_t column_offset, size_t array_size,
                                         const std::vector<uint8_t> &stream,
                                         size_t starting_src_pos) {
    const size_t data_type_size = sizeof(T);
    size_t curr_offset = column_offset;

    auto src_ptr = stream.data();
    std::advance(src_ptr, starting_src_pos);
    for (size_t index = 0; index < array_size; ++index) {
        T element;
        copy_swapped_bytes(reinterpret_cast<uint8_t *>(&element), src_ptr,
                           data_type_size);
        insert<T>(element, curr_offset);

        std::advance(src_ptr, data_type_size);
        curr_offset += data_type_size;
    }
}

//============================================================

// Caller has handled nulls.
void Row::insert_from_bigendian(const std::vector<uint8_t> &stream,
                                size_t starting_src_pos, const Data_Type &data_type,
                                const size_t &array_size, const size_t &offset,
                                const size_t &col_idx, bool dynamic_array_flag) {
    const size_t data_type_size(get_data_size(data_type));
    const bool is_bool(data_type == Data_Type::INT8_LE);

    if ((data_type_size == 1) && !is_bool) {
        const uint8_t *src_ptr = stream.data();
        src_ptr += starting_src_pos;

        // CHAR value is independent of endianness.
        // insert() loads dynamic_array_size.
        insert(src_ptr, src_ptr + array_size, offset, col_idx, dynamic_array_flag);
        return;
    }

    // In all other cases, load dynamic_array_size before inserting.
    if (dynamic_array_flag) {
        set_dynamic_array_size(col_idx, array_size);
    }

    // If we get here, either is_bool or data_type_size > 1.
    // In either case, process one array element at a time.
    switch (data_type_size) {
        case 1: {
            // is_bool
            size_t curr_offset = offset;
            auto src_iter = stream.begin();
            std::advance(src_iter, starting_src_pos);
            for (size_t index = 0; index != array_size; ++index) {
                if (*src_iter != 't' && *src_iter != 'T' && *src_iter != '1' &&
                    *src_iter != 'f' && *src_iter != 'F' && *src_iter != '0')
                    throw std::runtime_error(
                            "Invalid value for boolean.  The "
                            "integer representation is " +
                            std::to_string(*src_iter));
                insert((*src_iter == 't' || *src_iter == 'T' || *src_iter == '1')
                               ? static_cast<uint8_t>(1)
                               : static_cast<uint8_t>(0),
                       curr_offset);
                std::advance(src_iter, 1 /* data_type_size */);
                curr_offset += 1 /* data_type_size */;
            }
        } break;
        case 2: {
            // std::cout << "data_type_size = 2" << std::endl;
            insert_from_bigendian_internal<uint16_t>(offset, array_size, stream,
                                                     starting_src_pos);
        } break;
        case 4: {
            // std::cout << "data_type_size = 4" << std::endl;
            insert_from_bigendian_internal<uint32_t>(offset, array_size, stream,
                                                     starting_src_pos);
        } break;
        case 8: {
            // std::cout << "data_type_size = 8" << std::endl;
            insert_from_bigendian_internal<uint64_t>(offset, array_size, stream,
                                                     starting_src_pos);
        } break;
        default: {
            throw std::runtime_error("Invalid value for data_type_size: " +
                                     std::to_string(data_type_size));
        } break;
    }
}

}  // namespace tablator
