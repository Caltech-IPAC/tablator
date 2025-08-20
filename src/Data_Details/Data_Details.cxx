#include "../Data_Details.hxx"

#include "../Field_Framework.hxx"

namespace tablator {

Data_Details::Data_Details(const Field_Framework &field_framework, size_t num_rows)
        : Data_Details(field_framework.get_dynamic_col_idx_lookup(),
                       field_framework.get_row_size(), num_rows) {}

//==================================================================

void Data_Details::append_row(const Row &row) {
    assert(row.get_data().size() == get_row_size());
    assert(row.get_dynamic_array_sizes().size() == get_num_dynamic_columns());

    data_.insert(data_.end(), row.get_data().begin(), row.get_data().end());

    if (got_dynamic_columns()) {
        dynamic_array_sizes_by_row_.emplace_back();
        auto &new_sizes = dynamic_array_sizes_by_row_.back();
        new_sizes.insert(new_sizes.end(), row.get_dynamic_array_sizes().begin(),
                         row.get_dynamic_array_sizes().end());
    }
}

//==================================================================

// This function is called only by Table::append_rows(), which
// checks that the Field_Frameworks of the two tables are
// compatible.
void Data_Details::append_rows(const Data_Details &other) {
    assert(other.get_row_size() == get_row_size());

    data_.reserve(data_.size() + other.get_data().size());
    data_.insert(data_.end(), other.get_data().begin(), other.get_data().end());

    if (got_dynamic_columns()) {
        dynamic_array_sizes_by_row_.reserve(
                dynamic_array_sizes_by_row_.size() +
                other.get_dynamic_array_sizes_by_row().size());

        dynamic_array_sizes_by_row_.insert(
                dynamic_array_sizes_by_row_.end(),
                other.get_dynamic_array_sizes_by_row().begin(),
                other.get_dynamic_array_sizes_by_row().end());
    }
}

//==================================================================

void Data_Details::winnow_rows(const std::set<size_t> &selected_row_idx_list) {
    size_t num_selected_rows = selected_row_idx_list.size();
    size_t num_rows = get_num_rows();
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

    const auto data_start_ptr = data_.data();
    auto read_ptr = data_start_ptr;
    auto write_ptr = data_start_ptr;
    size_t write_idx = 0;
    size_t prev_row_idx = 0;

    for (auto row_idx : selected_row_idx_list) {
        if (row_idx >= write_idx) {
            if (row_idx > write_idx) {
                // Copy row_idx-th row to begin immediately after the end of the
                // previous copied row.
                read_ptr += ((row_idx - prev_row_idx) * row_size_);
                std::copy(read_ptr, read_ptr + row_size_, write_ptr);

                if (get_num_dynamic_columns() > 0) {
                    dynamic_array_sizes_by_row_.at(write_idx).swap(
                            dynamic_array_sizes_by_row_.at(row_idx));
                }
            }

            ++write_idx;
            write_ptr += row_size_;
            prev_row_idx = row_idx;
        }
    }

    // Delete all data past the last row copied.
    data_.resize(std::distance(data_start_ptr, write_ptr));

    if (got_dynamic_columns()) {
        dynamic_array_sizes_by_row_.resize(num_selected_rows);
    }
}


//==================================================================

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
    tablator::Row row(dest_ff);
    size_t cntr_col_idx = dest_ff.get_columns().size() - 1;
    uint64_t cntr(1);

    const auto *src_data_ptr = src_data.data();

    for (uint row_idx = 0; row_idx < num_rows; ++row_idx) {
        row.fill_with_zeros();

        // Copy null_bitflag column value from src_table.  (The
        // dest_column's null_bitfield flag is never set.)

        // Tell Row not to store dynamic info.  We'll do that manually.
        row.insert_data_only(src_data_ptr, src_data_ptr + src_nulls_size, 0);

        // Copy remaining column values from src_table starting from
        // end of dest_table's null_bitflag entry.
        row.insert_data_only(src_data_ptr + src_nulls_size, src_data_ptr + src_row_size,
                             dest_nulls_size);

        // Add the new column value.
        row.insert(cntr, dest_offsets[cntr_col_idx]);

        if (src_dd.got_dynamic_columns()) {
            // No need to update dynamic_array_sizes_per_row; just copy it over.
            row.set_dynamic_array_sizes(src_dd.get_dynamic_array_sizes(row_idx));
        }

        // Save it and prepare for next round.
        append_row(row);
        std::advance(src_data_ptr, src_row_size);
        ++cntr;
    }
}

//===========================================================

void Data_Details::combine_data_details(const Field_Framework &dest_ff,
                                        const Field_Framework &src1_ff,
                                        const Field_Framework &src2_ff,
                                        const Data_Details &src1_dd,
                                        const Data_Details &src2_dd) {
    const std::vector<Column> &dest_columns = dest_ff.get_columns();

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

    // Load dest_table data.
    size_t num_rows = src1_dd.get_num_rows();
    reserve_rows(num_rows);

    Row single_row(dest_ff);
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
        single_row.fill_with_zeros();

        size_t src1_row_start_offset = row_idx * src1_row_size;
        size_t src2_row_start_offset = row_idx * src2_row_size;

        uint8_t *row_data_ptr =
                reinterpret_cast<uint8_t *>(single_row.get_data().data());

        // Copy src1's null bitfield data for this row.
        memcpy(row_data_ptr, src1_data_ptr + src1_row_start_offset,
               src1_null_flags_size);

        if (num_src1_columns % 8 == 1) {  // i.e. (num_visible_columns % 8) == 0
            // Starting where previous memcpy left off, copy src2's null bitfield data
            // for this row.
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

        // Populate row.dynamic_array_sizes_ as needed.
        if (src1_dd.got_dynamic_columns() || src2_dd.got_dynamic_columns()) {
            std::vector<uint32_t> &row_dynamic_array_sizes =
                    single_row.get_dynamic_array_sizes();

            if (src1_dd.got_dynamic_columns()) {
                // Get dynamic array sizes from src1_table...
                const std::vector<uint32_t> &src1_dynamic_array_sizes =
                        src1_dd.get_dynamic_array_sizes_by_row().at(row_idx);


                row_dynamic_array_sizes.insert(row_dynamic_array_sizes.end(),
                                               src1_dynamic_array_sizes.begin(),
                                               src1_dynamic_array_sizes.end());
            }

            if (src2_dd.got_dynamic_columns()) {
                // ... and from src2_table.

                const std::vector<uint32_t> &src2_dynamic_array_sizes =
                        src2_dd.get_dynamic_array_sizes_by_row().at(row_idx);


                row_dynamic_array_sizes.insert(row_dynamic_array_sizes.end(),
                                               src2_dynamic_array_sizes.begin(),
                                               src2_dynamic_array_sizes.end());
            }
        }
        // Make single_row official
        append_row(single_row);
    }
}

}  // namespace tablator
