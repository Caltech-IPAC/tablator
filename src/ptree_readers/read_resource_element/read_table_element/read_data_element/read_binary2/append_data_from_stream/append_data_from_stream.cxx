#include "../../../../../../ptree_readers.hxx"

#include <boost/spirit/include/qi.hpp>

#include "../../../../../../Data_Details.hxx"
#include "../../../../../../Field_Framework.hxx"
#include "../../../../../../Utils/Null_Utils.hxx"
#include "../../../../../../Utils/Table_Utils.hxx"

namespace tablator {

// for dynamic array_size column
void insert_swapped(Row &row, size_t column_offset, const Data_Type &data_type,
                    size_t array_size, const std::vector<uint8_t> &stream,
                    size_t old_position);

// for fixed array_size column
inline void insert_swapped(Row &row, size_t column_offset, const Column &column,
                           const std::vector<uint8_t> &stream, size_t old_position) {
    return insert_swapped(row, column_offset, column.get_type(),
                          column.get_array_size(), stream, old_position);
}

//===========================================================


void ptree_readers::append_data_from_stream(Data_Details &data_details,
                                            const Field_Framework &field_framework,
                                            const std::vector<uint8_t> &stream,
                                            const std::vector<Field> &fields,
                                            size_t num_rows) {
    auto &columns = field_framework.get_columns();
    auto &offsets = field_framework.get_offsets();

    auto &data = data_details.get_data();

    const size_t null_flags_size((columns.size() + 6) / 8);
    size_t src_pos(0);

    Row single_row(field_framework.get_row_size());
    for (size_t r = 0; r < num_rows; ++r) {
        single_row.fill_with_zeros();
        size_t row_offset(src_pos);
        src_pos += null_flags_size;
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            auto column = columns.at(col_idx);
            auto col_array_size = column.get_array_size();
            auto col_type = column.get_type();

            bool dynamic_array_flag = fields[col_idx].get_dynamic_array_flag();
            if (is_null_MSB(stream, row_offset, col_idx)) {
                single_row.insert_null(col_type, col_array_size, col_idx,
                                       offsets[col_idx], offsets[col_idx + 1]);
                if (dynamic_array_flag) {
                    src_pos += sizeof(uint32_t);
                } else {
                    src_pos += data_size(col_type) * col_array_size;
                }
            } else if (dynamic_array_flag) {
                auto begin = stream.begin();
                std::advance(begin, src_pos);
                auto end = stream.begin();
                src_pos += sizeof(uint32_t);
                std::advance(end, src_pos);
                uint32_t dynamic_array_size(0);

                // Parse big-ended dynamic_array_size and write it
                // to <row> (and eventually to <data>)
                // little-endedly, since <data> is for internal
                // use only.  We'll swap back if we later write
                // this table in binary2 format.

                boost::spirit::qi::parse(begin, end, boost::spirit::qi::big_dword,
                                         dynamic_array_size);

                memcpy(single_row.get_data().data() + offsets[col_idx],
                       &dynamic_array_size, sizeof(uint32_t));

                // Now write the array itself, again swapping from
                // big-ended to little-ended for internal use.
                insert_swapped(single_row, offsets[col_idx] + sizeof(uint32_t),
                               col_type, dynamic_array_size, stream, src_pos);
                src_pos += data_size(col_type) * dynamic_array_size;
            } else {
                insert_swapped(single_row, offsets[col_idx], column, stream, src_pos);
                src_pos += col_array_size * data_size(col_type);
            }

        }  // end loop through columns
        if (src_pos <= stream.size()) {
            data_details.append_row(single_row);
        }
    }  // end loop through rows
}
}  // namespace tablator
