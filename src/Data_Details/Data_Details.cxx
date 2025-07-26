#include "../Data_Details.hxx"

#include "../Field_Framework.hxx"

namespace tablator {

Data_Details::Data_Details(const Field_Framework &field_framework, size_t num_rows)
        : Data_Details(field_framework.get_row_size(), num_rows) {}

void Data_Details::add_cntr_column(const Field_Framework &dest_ff,
                                   const Field_Framework &src_ff,
                                   const Data_Details &src_dd) {
    const auto &src_offsets = src_ff.get_offsets();
    size_t src_nulls_size(src_offsets.at(1));
    size_t src_row_size = src_dd.get_row_size();
    size_t num_rows = src_dd.get_num_rows();
    const auto &src_data = src_dd.get_data();

    const auto &dest_offsets = dest_ff.get_offsets();
    size_t dest_nulls_size(dest_offsets.at(1));

    reserve_rows(num_rows);
    size_t cntr_col_idx = dest_ff.get_columns().size() - 1;

    tablator::Row row(dest_ff);
    uint64_t cntr(1);

    for (const auto *row_pointer = src_data.data();
         row_pointer < src_data.data() + src_data.size();
         row_pointer += src_row_size, ++cntr) {
        row.fill_with_zeros();
        // Copy null_bitflag column value from src_table.
        // The dest_column's null_bitfield flag is never set.
        row.insert(row_pointer, row_pointer + src_nulls_size, 0);

        // Copy remaining column values from src_table starting from
        // end of dest_table's null_bitflag entry.
        row.insert(row_pointer + src_nulls_size, row_pointer + src_row_size,
                   dest_nulls_size);

        // Add the new column value.
        row.insert(cntr, dest_offsets[cntr_col_idx]);

        append_row(row);
    }
}

//===========================================================

void Data_Details::combine_data_details(const Field_Framework &dest_ff,
                                        const Field_Framework &src1_ff,
                                        const Field_Framework &src2_ff,
                                        const Data_Details &src1_dd,
                                        const Data_Details &src2_dd) {
    // Prepare to load dest_table data.
    size_t src1_null_flags_size = src1_ff.get_offsets().at(1);
    size_t src2_null_flags_size = src2_ff.get_offsets().at(1);
    size_t dest_null_flags_size = dest_ff.get_offsets().at(1);

    size_t num_src1_columns = src1_ff.get_columns().size();
    size_t num_src2_columns = src2_ff.get_columns().size();

    size_t src1_row_size = src1_ff.get_row_size();
    size_t src2_row_size = src2_ff.get_row_size();

    const uint8_t *src1_data_ptr = src1_dd.get_data().data();
    const uint8_t *src2_data_ptr = src2_dd.get_data().data();

    // Load data.
    size_t num_rows = src1_dd.get_num_rows();
    reserve_rows(num_rows);

    Row curr_row(dest_ff);
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
        curr_row.fill_with_zeros();

        size_t src1_row_start_offset = row_idx * src1_row_size;
        size_t src2_row_start_offset = row_idx * src2_row_size;

        uint8_t *row_data_ptr = reinterpret_cast<uint8_t *>(curr_row.get_data().data());

        // Copy src1's null bitfield data for this row.
        memcpy(row_data_ptr, src1_data_ptr + src1_row_start_offset,
               src1_null_flags_size);

        // Starting where previous memcpy left off, copy src2's null bitfield data for
        // this row.
        if (num_src1_columns % 8 == 1) {  // i.e. (num_visible_columns % 8) == 0
            memcpy(row_data_ptr + src1_null_flags_size,
                   src2_data_ptr + src2_row_start_offset, src2_null_flags_size);
        } else {
            // Handle null bits one by one; memcpy won't let us start copying at
            // mid-byte.
            for (uint src2_col_idx = 1; src2_col_idx < num_src2_columns;
                 ++src2_col_idx) {
                size_t active_byte_in_src2_row = (src2_col_idx - 1) / 8;
                size_t active_bit_in_src2_row = (src2_col_idx - 1) % 8;
                const char src2_mask = (128 >> active_bit_in_src2_row);

                if (src2_data_ptr[src2_row_start_offset + active_byte_in_src2_row] &
                    src2_mask) {
                    size_t active_byte_in_dest_row =
                            (num_src1_columns - 1 + src2_col_idx - 1) / 8;
                    size_t active_bit_in_dest_row =
                            (num_src1_columns - 1 + src2_col_idx - 1) % 8;
                    const char dest_mask = (128 >> active_bit_in_dest_row);
                    row_data_ptr[active_byte_in_dest_row] |= dest_mask;
                }
            }
        }

        // Skip past dest_table's null_bitfield column and copy src1_table's
        // visible-column data for this row.
        memcpy(row_data_ptr + dest_null_flags_size,
               src1_data_ptr + src1_row_start_offset + src1_null_flags_size,
               src1_row_size - src1_null_flags_size);

        // Start where previous copy left off and copy src2_table's visible-column data
        // for this row.
        memcpy(row_data_ptr + dest_null_flags_size + src1_row_size -
                       src1_null_flags_size,
               src2_data_ptr + src2_row_start_offset + src2_null_flags_size,
               src2_row_size - src2_null_flags_size);

        // Make curr_row official
        append_row(curr_row);
    }
}

//===========================================================

void Data_Details::winnow_rows(const std::set<size_t> &selected_row_idx_list) {
    size_t num_rows = get_num_rows();
    size_t num_selected_rows = selected_row_idx_list.size();

    if (num_selected_rows > num_rows) {
        throw std::runtime_error("Number of selected rows must not exceed " +
                                 std::to_string(num_rows));
    }
    if (*selected_row_idx_list.rbegin() >= num_rows) {
        throw std::runtime_error("invalid row index: " +
                                 std::to_string(*selected_row_idx_list.rbegin()));
    }
    if (num_selected_rows == num_rows) {
        return;
    }

    size_t row_size = get_row_size();
    const auto data_start_ptr = get_data().data();
    auto read_ptr = data_start_ptr;
    auto write_ptr = data_start_ptr;
    size_t write_idx = 0;
    size_t prev_row_idx = 0;

    for (auto row_idx : selected_row_idx_list) {
        read_ptr += ((row_idx - prev_row_idx) * row_size);
        if (row_idx > write_idx) {
            // Copy row_idx-th row to begin immediately after the end of the
            // previous copied row.
            std::copy(read_ptr, read_ptr + row_size, write_ptr);
        }
        ++write_idx;
        write_ptr += row_size;
        prev_row_idx = row_idx;
    }

    // Delete all data past the last row copied.
    get_data().resize(std::distance(data_start_ptr, write_ptr));
}

}  // namespace tablator
