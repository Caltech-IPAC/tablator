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
    dest_table.get_data_details().add_cntr_column(dest_table.get_field_framework(),
                                                  src_table.get_field_framework(),
                                                  src_table.get_data_details());

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

    // Construct dest_table.
    Table dest_table(combined_columns, false /* got_null_bitfields_flag */, num_rows);

    dest_table.get_data_details().combine_data_details(
            dest_table.get_field_framework(), src1_table.get_field_framework(),
            src2_table.get_field_framework(), src1_table.get_data_details(),
            src2_table.get_data_details());

    return dest_table;
}

}  // namespace tablator
