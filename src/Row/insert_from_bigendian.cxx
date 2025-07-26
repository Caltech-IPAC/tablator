#include "../Row.hxx"

#include <stdexcept>

#include "../data_size.hxx"

namespace {
template <class T, class Rule>
T parse_bigendian_element(const Rule &rule,
                          std::vector<uint8_t>::const_iterator &src_iter) {
    T element = 0;
    boost::spirit::qi::parse(src_iter, src_iter + sizeof(T), rule, element);
    return element;
}

}  // namespace


namespace tablator {

template <class T, class Rule>
void Row::insert_from_bigendian_internal(size_t column_offset, const Rule &rule,
                                         size_t array_size,
                                         const std::vector<uint8_t> &stream,
                                         size_t starting_src_pos) {
    const size_t data_type_size = sizeof(T);
    size_t curr_offset = column_offset;

    auto src_iter = stream.begin();
    std::advance(src_iter, starting_src_pos);

    for (size_t index = 0; index < array_size; ++index) {
        insert<T>(parse_bigendian_element<T>(rule, src_iter), curr_offset);

        std::advance(src_iter, data_type_size);
        curr_offset += data_type_size;
    }
}

//============================================================

void Row::insert_from_bigendian(const std::vector<uint8_t> &stream,
                                size_t starting_src_pos, const Data_Type &data_type,
                                const size_t &array_size, const size_t &offset) {
    const size_t data_type_size(data_size(data_type));
    const bool is_bool(data_type == Data_Type::INT8_LE);

    if ((data_type_size == 1) && !is_bool) {
        const uint8_t *src_ptr = stream.data();
        src_ptr += starting_src_pos;

        // CHAR value is independent of endianness.
        insert(src_ptr, src_ptr + array_size, offset);
        return;
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
            insert_from_bigendian_internal<uint16_t>(
                    offset, boost::spirit::qi::big_word, array_size, stream,
                    starting_src_pos);
        } break;
        case 4: {
            insert_from_bigendian_internal<uint32_t>(
                    offset, boost::spirit::qi::big_dword, array_size, stream,
                    starting_src_pos);
        } break;
        case 8: {
            insert_from_bigendian_internal<uint64_t>(
                    offset, boost::spirit::qi::big_qword, array_size, stream,
                    starting_src_pos);
        } break;
        default: {
            throw std::runtime_error("Invalid value for data_type_size: " +
                                     std::to_string(data_type_size));
        } break;
    }
}

}  // namespace tablator
