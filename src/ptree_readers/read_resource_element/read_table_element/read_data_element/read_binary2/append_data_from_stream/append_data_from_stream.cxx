#include "../../../../../../ptree_readers.hxx"

#include <boost/spirit/include/qi.hpp>

#include "../../../../../../Data_Details.hxx"
#include "../../../../../../Field_Framework.hxx"
#include "../../../../../../Row.hxx"
#include "../../../../../../Utils/Null_Utils.hxx"
#include "../../../../../../Utils/Table_Utils.hxx"

namespace tablator {

void ptree_readers::append_data_from_stream(Data_Details &data_details,
                                            const Field_Framework &field_framework,
                                            const std::vector<uint8_t> &stream,
                                            const std::vector<Field> &fields,
                                            size_t num_rows) {
    const auto &columns = field_framework.get_columns();
    const auto &offsets = field_framework.get_offsets();
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
            auto curr_array_size = col_array_size;
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
            } else {
                if (dynamic_array_flag) {
                    auto size_begin = stream.begin();
                    std::advance(size_begin, src_pos);
                    auto size_end = stream.begin();
                    src_pos += sizeof(uint32_t);
                    std::advance(size_end, src_pos);

                    // Extract dynamic_array_size, swapping from big-ended
                    // to little-ended for internal use.
                    uint32_t dynamic_array_size(0);
                    boost::spirit::qi::parse(size_begin, size_end,
                                             boost::spirit::qi::big_dword,
                                             dynamic_array_size);
                    if (dynamic_array_size > col_array_size) {
                        throw std::runtime_error(
                                "dynamic_array_size > col_array_size (" +
                                std::to_string(dynamic_array_size) + " > " +
                                std::to_string(col_array_size) + ")");
                    }
                    curr_array_size = dynamic_array_size;
                }
                // Now write the array itself, again swapping from
                // big-ended to little-ended for internal use.
                single_row.insert_from_bigendian(stream, src_pos, col_type,
                                                 curr_array_size,
                                                 offsets[col_idx]);
                src_pos += data_size(col_type) * curr_array_size;
            }
        }  // end loop through columns
        if (src_pos <= stream.size()) {
            data_details.append_row(single_row);
        }
    }  // end loop through rows
}
}  // namespace tablator
