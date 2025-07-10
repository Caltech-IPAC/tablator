#include "Table_Ops.hxx"

#include <set>

#include "Row.hxx"
#include "Table.hxx"

//==================================================================

namespace tablator {
//============================================================================

Table add_counter_column(const Table &src_table, const std::string &col_name) {
    const auto &src_columns = src_table.get_columns();
    size_t num_src_columns = src_columns.size();

    std::vector<tablator::Column> dest_columns;

    for (size_t col_idx = 1; col_idx < num_src_columns; ++col_idx) {
        if (src_columns[col_idx].get_name() == col_name) {
            throw std::runtime_error(
                    "src table already has column with indicated name.");
        }
        dest_columns.push_back(src_columns[col_idx]);
    }
    // Append the new column.
    dest_columns.emplace_back(col_name, Data_Type::UINT64_LE, 1 /* array_size */,
                              false /* dynamic_array_flag */);

    tablator::Table dest_table(dest_columns, false /* got_null_bitfields_column */);
    const auto &src_offsets = src_table.get_offsets();
    const auto &dest_offsets = dest_table.get_offsets();


    size_t src_nulls_size(src_offsets.at(1));
    size_t dest_nulls_size(dest_offsets.at(1));
    size_t src_row_size = src_table.get_row_size();
    size_t dest_row_size = dest_table.get_row_size();
    size_t num_rows = src_table.get_num_rows();

    // JTODO Data_Details function to handle this?
    const auto &src_data = src_table.get_data_details().get_data();

    auto &dest_data_details = dest_table.get_data_details();
    dest_data_details.reserve_rows(num_rows);
    auto &dest_data = dest_data_details.get_data();

    tablator::Row row(dest_row_size);
    uint64_t cntr(1);

    for (const auto *row_pointer = src_data.data();
         row_pointer < src_data.data() + src_data.size();
         row_pointer += src_row_size, ++cntr) {
        row.fill_with_zeros();
        // Copy null_bitflag column valuefrom src_table.
        // The dest_column's null_bitfield flag is never set.
        row.insert(row_pointer, row_pointer + src_nulls_size, 0);


        // Copy remaining column values from src_table starting from
        // end of dest_table's null_bitflag entry.
        row.insert(row_pointer + src_nulls_size, row_pointer + src_row_size,
                   dest_nulls_size);
        // Add the new column value.
        row.insert(cntr, dest_offsets[num_src_columns]);

        dest_table.append_row(row);
    }
    return dest_table;
}


//=====================================================================

// Src tables must have the same number of rows.
// Dest table's set of columns consists of src1's columns followed by src2's columns.
Table combine_tables(const Table &src1_table, const Table &src2_table) {
    size_t num_rows = src1_table.get_num_rows();
    if (src2_table.get_num_rows() != num_rows) {
        throw std::runtime_error("src tables have different numbers of rows.");
    }
    const std::vector<Column> &src1_columns = src1_table.get_columns();
    size_t num_src1_columns = src1_columns.size();

    const std::vector<Column> &src2_columns = src2_table.get_columns();
    size_t num_src2_columns = src2_columns.size();


    std::set<std::string> src1_visible_column_names;
    for (size_t col_idx = 1; col_idx < num_src1_columns; ++col_idx) {
        src1_visible_column_names.insert(src1_columns.at(col_idx).get_name());
    }

    if (src1_visible_column_names.size() != num_src1_columns - 1) {
        throw std::runtime_error("src table #1 column names are not distinct.");
    }

    std::set<std::string> src2_visible_column_names;
    for (size_t col_idx = 1; col_idx < num_src2_columns; ++col_idx) {
        src2_visible_column_names.insert(src2_columns.at(col_idx).get_name());
    }

    if (src2_visible_column_names.size() != num_src2_columns - 1) {
        throw std::runtime_error("src table #2 column names are not distinct.");
    }

    std::set<std::string> dest_visible_column_names;
    dest_visible_column_names.insert(src1_visible_column_names.begin(),
                                     src1_visible_column_names.end());
    dest_visible_column_names.insert(src2_visible_column_names.begin(),
                                     src2_visible_column_names.end());

    if (dest_visible_column_names.size() != num_src1_columns + num_src2_columns - 2) {
        throw std::runtime_error(
                "src tables have at least one column name in common: ");
    }

    std::vector<Column> combined_columns;
    combined_columns.reserve(num_src1_columns + num_src2_columns - 2);

    // Load combined_columns with visible columns (all but null-flag one) of src1
    for (size_t col_idx = 1; col_idx < num_src1_columns; ++col_idx) {
        combined_columns.emplace_back(src1_columns[col_idx]);
    }

    // Same for columns of src2_table.
    for (size_t col_idx = 1; col_idx < num_src2_columns; ++col_idx) {
        combined_columns.emplace_back(src2_columns[col_idx]);
    }

    // JTODO Add lower-level functions to handle the copying?

    // Construct dest_table.
    Table dest_table(combined_columns, false /* got_null_bitfields_flag */, num_rows);
    std::vector<Column> &dest_columns = dest_table.get_columns();

    // Prepare to load dest_table data.
    const std::vector<size_t> &src1_offsets = src1_table.get_offsets();
    const std::vector<size_t> &src2_offsets = src2_table.get_offsets();
    const std::vector<size_t> &dest_offsets = dest_table.get_offsets();

    size_t src1_null_flags_size = src1_offsets.at(1);
    size_t src2_null_flags_size = src2_offsets.at(1);
    size_t dest_null_flags_size = dest_offsets.at(1);


    size_t src1_row_size = src1_table.get_row_size();
    size_t src2_row_size = src2_table.get_row_size();
    size_t dest_row_size = dest_table.get_row_size();

    const uint8_t *src1_data_ptr = src1_table.get_data().data();
    const uint8_t *src2_data_ptr = src2_table.get_data().data();

    // Load dest_table data.

    Row curr_row(dest_row_size);
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
        dest_table.get_data_details().append_row(curr_row);
    }
    return dest_table;
}

}  // namespace tablator
