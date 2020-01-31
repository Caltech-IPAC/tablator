#include "../../../../../../ptree_readers.hxx"

#include <boost/spirit/include/qi.hpp>

namespace {
template <class T, class Rule>
void parse_and_insert_element(const Rule &rule,
                              std::vector<uint8_t>::const_iterator &begin,
                              std::vector<uint8_t>::const_iterator &end,
                              const size_t &offset, tablator::Row &row) {
    T result;
    boost::spirit::qi::parse(begin, end, rule, result);
    row.insert(result, offset);
}

void insert_swapped_element(std::vector<uint8_t>::const_iterator &begin,
                            std::vector<uint8_t>::const_iterator &end,
                            const size_t &data_type_size, const size_t &offset,
                            const bool &is_bool, tablator::Row &row) {
    switch (data_type_size) {
        case 1:
            // boolean serialization is a little different from just using 0
            // or 1.  Instead, it uses a serialization of the character
            // representation of true or false
            if (is_bool) {
                if (*begin != 't' && *begin != 'T' && *begin != '1' && *begin != 'f' &&
                    *begin != 'F' && *begin != '0')
                    throw std::runtime_error(
                            "Invalid value for boolean.  The "
                            "integer representation is " +
                            std::to_string(*begin));
                row.insert((*begin == 't' || *begin == 'T' || *begin == '1')
                                   ? static_cast<uint8_t>(1)
                                   : static_cast<uint8_t>(0),
                           offset);
            } else {
                row.insert(*begin, offset);
            }
            break;
        case 2:
            parse_and_insert_element<uint16_t>(boost::spirit::qi::big_word, begin, end,
                                               offset, row);
            break;
        case 4:
            parse_and_insert_element<uint32_t>(boost::spirit::qi::big_dword, begin, end,
                                               offset, row);
            break;
        case 8:
            parse_and_insert_element<uint64_t>(boost::spirit::qi::big_qword, begin, end,
                                               offset, row);
            break;
        default:
            throw std::runtime_error("Invalid value for data_type_size: " +
                                     std::to_string(data_type_size));
            break;
    }
}
}  // namespace

namespace tablator {
void insert_swapped(const size_t &column_offset, const Data_Type &data_type,
                    const size_t &array_size, const std::vector<uint8_t> &stream,
                    const size_t &starting_position, Row &row) {
    const size_t data_type_size(data_size(data_type));
    const bool is_bool(data_type == Data_Type::INT8_LE);
    size_t position = starting_position;
    for (size_t index = 0; index != array_size; ++index) {
        auto begin = stream.begin();
        std::advance(begin, position);
        auto end = stream.begin();
        position += data_type_size;
        std::advance(end, position);

        insert_swapped_element(begin, end, data_type_size,
                               column_offset + index * data_type_size, is_bool, row);
    }
}
}  // namespace tablator
