#pragma once

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "Command_Line_Options.hxx"
#include "Data_Type.hxx"

namespace tablator {
class Table;

static constexpr char const *WHITESPACE = " \n\r\t\f\v";
static constexpr char const *NEWLINES = "\n\r";

class Ipac_Table_Writer {
public:
    /*******************************************/
    /* Table-writer functions */
    /*******************************************/

    static void write(const Table &table, std::ostream &os);

    static void write_keywords_and_comments(const Table &table, std::ostream &os);

    static void write_column_headers(const Table &table, std::ostream &os);
    static void write_subtable_by_row(const Table &table, std::ostream &os,
                                      const std::vector<size_t> &requested_row_ids,
                                      const Command_Line_Options &options = default_options);
    static void write_subtable_by_column_and_row(
            const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
            const std::vector<size_t> &requested_row_ids,
            const Command_Line_Options &options = default_options);

    static void write_subtable_by_row(const Table &table, std::ostream &os,
                                      size_t start_row, size_t row_count,
                                      const Command_Line_Options &options = default_options);
    static void write_subtable_by_column_and_row(
            const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
            size_t start_row, size_t row_count,
            const Command_Line_Options &options = default_options);

    static void write_subtable_by_column_and_row(
            const Table &table, std::ostream &os, const std::vector<size_t> &column_ids,
            const Command_Line_Options &options = default_options);

    static void write_single_record(const Table &table, std::ostream &os,
                                    size_t row_idx);
    static void write_single_record(const Table &table, std::ostream &os,
                                    const std::vector<size_t> &column_ids,
                                    size_t row_idx);

    static void write_consecutive_records(const Table &table, std::ostream &os,
                                          size_t start_row, size_t row_count);
    static void write_consecutive_records(const Table &table, std::ostream &os,
                                          const std::vector<size_t> &column_ids,
                                          size_t start_row, size_t row_count);
    static void write_selected_records(const Table &table, std::ostream &os,
                                       const std::vector<size_t> &requested_row_ids);

    static void write_selected_records(const Table &table, std::ostream &os,
                                       const std::vector<size_t> &column_ids,
                                       const std::vector<size_t> &requested_row_ids);

    /*******************************************/
    /* Auxiliary functions exposed through Table */
    /*******************************************/

    static std::string to_ipac_string(const Data_Type &type);
    static std::vector<size_t> get_column_widths(const Table &table);

    /*******************************************/
    /* Helper function  */
    /*******************************************/

    static bool is_valid_col_idx(const Table &table, size_t col_idx);

private:
    static size_t get_true_row_count(const Table &table, size_t start_row,
                                     size_t requested_row_count);

    static void write_keywords_and_comments(const Table &table, std::ostream &os,
                                            const std::vector<size_t> &included_col_ids,
                                            size_t num_rows_to_report);

    static void write_keywords_and_comments(const Table &table, std::ostream &os,
                                            size_t num_rows_to_report);

    static void write_keywords_and_comments(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids);

    static size_t get_single_column_width(const Table &table,
                                          const std::vector<size_t> &requested_row_ids,
                                          size_t col_idx);

    static std::vector<size_t> get_column_widths(
            const Table &table, const std::vector<size_t> &requested_row_ids,
            const std::vector<size_t> &requested_col_ids);

    static void write_column_headers(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_column_headers(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);


    static void write_subtable_by_row(
            const Table &table, std::ostream &os, size_t start_row, size_t row_count,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing,
            const Command_Line_Options &options = default_options);

    static void write_subtable_by_column_and_row(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids, size_t start_row,
            size_t row_count, const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing,
            const Command_Line_Options &options = default_options);

    static void write_subtable_by_row(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &requested_row_ids,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing,
            const Command_Line_Options &options = default_options);

    static void write_subtable_by_column_and_row(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids,
            const std::vector<size_t> &requested_row_ids,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing,
            const Command_Line_Options &options = default_options);

    static void write_single_record(
            const Table &table, std::ostream &os, size_t row_offset,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_single_record(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids, size_t row_offset,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_consecutive_records(
            const Table &table, std::ostream &os, size_t start_row, size_t row_count,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_consecutive_records(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids, size_t start_row,
            size_t row_count, const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_selected_records(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &requested_row_ids,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_selected_records(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids,
            const std::vector<size_t> &requested_row_ids,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);


    // The functions below have no public counterpart.
    static void write_single_value(const Table &table, std::ostream &os,
                                   size_t column_id, size_t row_offset, size_t width,
                                   const std::vector<Data_Type> &datatypes_for_writing);

    static void write_single_record_by_offset(
            const Table &table, std::ostream &os, size_t row_offset,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_single_record_by_offset(
            const Table &table, std::ostream &os,
            const std::vector<size_t> &included_column_ids, size_t row_offset,
            const std::vector<size_t> &ipac_column_widths,
            const std::vector<Data_Type> &datatypes_for_writing);

    static size_t write_column_name(const Table &table, std::ostream &os, size_t col_id,
                                    size_t col_width, size_t effective_array_size);

    static void write_column_type(const Table &table, std::ostream &os,
                                  const std::vector<Data_Type> &datatypes_for_writing,
                                  size_t col_id, size_t col_width);

    static void write_column_unit(const Table &table, std::ostream &os, size_t col_id,
                                  size_t col_width, size_t effective_array_size);

    static void write_column_null(const Table &table, std::ostream &os, size_t col_id,
                                  size_t col_width, size_t effective_array_size);
};

}  // namespace tablator
