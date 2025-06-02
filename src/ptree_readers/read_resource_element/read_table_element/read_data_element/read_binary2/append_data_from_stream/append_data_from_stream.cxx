#include "../../../../../../ptree_readers.hxx"
#include "../is_null_MSb.hxx"

#include <boost/spirit/include/qi.hpp>

#include "../../../../../../Utils/Table_Utils.hxx"

namespace tablator {

/**********************************************************/
/*  Convert from  little-endian to big-endian             */
/**********************************************************/
// JTODO put this someplace central

  // Copy data_size bytes in reverse order from sec to dest.
void swap_copy(uint8_t *dest, const uint8_t *src, size_t data_size) {
    for (size_t i = 0; i < data_size; i++) {
        dest[data_size - 1 - i] = src[i];
    }
}



void insert_swapped(const size_t &column_offset, const Data_Type &data_type,
                    const size_t &array_size, const std::vector<uint8_t> &stream,
                    const size_t &old_position, Row &row);
inline void insert_swapped(const size_t &column_offset, const Column &column,
                           const std::vector<uint8_t> &stream,
                           const size_t &old_position, Row &row) {
    return insert_swapped(column_offset, column.get_type(), column.get_array_size(),
                          stream, old_position, row);
}

void ptree_readers::append_data_from_stream(
        std::vector<uint8_t> &data, const std::vector<Column> &columns,
        const std::vector<size_t> &offsets, const std::vector<uint8_t> &stream,
        const std::vector<ptree_readers::Field_And_Flag> &field_flag_pairs,
        size_t num_rows) {
    const size_t null_flags_size((columns.size() + 6) / 8);
    size_t position(0);
    Row row(get_row_size(offsets));
    for (size_t r = 0; r < num_rows; ++r) {
        row.fill_with_zeros();
        size_t row_offset(position);
        position += null_flags_size;
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            bool dynamic_array_flag = field_flag_pairs[col_idx].get_dynamic_array_flag();
            if (is_null_MSb(stream, row_offset, col_idx)) {
                row.set_null(columns[col_idx].get_type(),
                             columns[col_idx].get_array_size(), col_idx,
                             offsets[col_idx], offsets[col_idx + 1]);
                if (dynamic_array_flag) {
                    position += sizeof(uint32_t);
                } else {
                    position += data_size(columns[col_idx].get_type()) *
                                columns[col_idx].get_array_size();
                }
            } else {
                if (dynamic_array_flag) {
                    auto begin = stream.begin();
                    std::advance(begin, position);
                    auto end = stream.begin();
                    position += sizeof(uint32_t);
                    std::advance(end, position);
                    uint32_t dynamic_array_size(0);
                    boost::spirit::qi::parse(begin, end, boost::spirit::qi::big_dword,
                                             dynamic_array_size);

					// JTODO or just memcpy?

					//					swap_copy(row.get_data(), &dynamic_array_size, sizeof(uint32_t));
#if 0
					insert_swapped(offsets[col_idx], Data_Type::UINT32_LE, 1, &dynamic_array_size, 0, row);

#else
					//					swap_copy(row.get_data() + offsets[col_idx], &dynamic_array_size, sizeof(uint32_t));
#if 0
					auto row_insert_ptr = row.get_data().data();
					advance(row_insert_ptr, offsets[col_idx]);
#endif
					memcpy(row.get_data().data() + offsets[col_idx], &dynamic_array_size, sizeof(uint32_t));
#endif
					// std::cout << "between insert_swapped() for dynamic, size: " << dynamic_array_size << std::endl;

                    insert_swapped(offsets[col_idx] + sizeof(uint32_t), columns[col_idx].get_type(),
                                   dynamic_array_size, stream, position, row);
                    position +=
                            data_size(columns[col_idx].get_type()) * dynamic_array_size;
                } else {
                    insert_swapped(offsets[col_idx], columns[col_idx], stream, position,
                                   row);
                    position += columns[col_idx].get_array_size() *
                                data_size(columns[col_idx].get_type());
                }
            }
        }  // end loop through columns
        if (position <= stream.size()) {
            append_row(data, row);
        }
    }
}
}  // namespace tablator
