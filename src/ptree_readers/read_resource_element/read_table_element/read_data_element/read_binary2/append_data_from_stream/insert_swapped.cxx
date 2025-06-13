#include "../../../../../../ptree_readers.hxx"

#include <boost/spirit/include/qi.hpp>

#include "../../../../../../Utils/Table_Utils.hxx"

namespace {
template <class T, class Rule>
void parse_and_insert_element(tablator::Row &row, size_t offset, const Rule &rule,
                              std::vector<uint8_t>::const_iterator &src_begin,
                              std::vector<uint8_t>::const_iterator &src_end) {
    T result;
    boost::spirit::qi::parse(src_begin, src_end, rule, result);
    row.insert(result, offset);
}

void insert_swapped_element(tablator::Row &row, size_t offset, std::vector<uint8_t>::const_iterator &src_begin,
                            std::vector<uint8_t>::const_iterator &src_end,
                            size_t data_type_size, 
                            const bool &is_bool) {
    switch (data_type_size) {
        case 1:
            // boolean serialization is a little different from just using 0
            // or 1.  Instead, it uses a serialization of the character
            // representation of true or false
            if (is_bool) {
                if (*src_begin != 't' && *src_begin != 'T' && *src_begin != '1' && *src_begin != 'f' &&
                    *src_begin != 'F' && *src_begin != '0')
                    throw std::runtime_error(
                            "Invalid value for boolean.  The "
                            "integer representation is " +
                            std::to_string(*src_begin));
                row.insert((*src_begin == 't' || *src_begin == 'T' || *src_begin == '1')
                                   ? static_cast<uint8_t>(1)
                                   : static_cast<uint8_t>(0),
                           offset);
            } else {
                row.insert(*src_begin, offset);
            }
            break;
        case 2:
		  parse_and_insert_element<uint16_t>(row, offset, boost::spirit::qi::big_word, src_begin, src_end);
            break;
        case 4:
		  parse_and_insert_element<uint32_t>(row, offset, boost::spirit::qi::big_dword, src_begin, src_end);
            break;
        case 8:
		  parse_and_insert_element<uint64_t>(row, offset, boost::spirit::qi::big_qword, src_begin, src_end);
            break;
        default:
            throw std::runtime_error("Invalid value for data_type_size: " +
                                     std::to_string(data_type_size));
            break;
    }
}
}  // namespace

namespace tablator {
  void insert_swapped(Row &row, size_t column_offset, const Data_Type &data_type,
                    size_t array_size, const std::vector<uint8_t> &stream,
                    size_t starting_src_pos) {
    const size_t data_type_size(data_size(data_type));
    const bool is_bool(data_type == Data_Type::INT8_LE);
    size_t src_pos = starting_src_pos;
    for (size_t index = 0; index != array_size; ++index) {
        auto src_begin = stream.begin();
        std::advance(src_begin, src_pos);
        auto src_end = stream.begin();
        src_pos += data_type_size;
        std::advance(src_end, src_pos);

        insert_swapped_element(row, column_offset + index * data_type_size, src_begin, src_end, data_type_size,
                               is_bool);
    }
}
}  // namespace tablator
