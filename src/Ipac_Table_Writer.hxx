#pragma once

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "Data_Type.hxx"

namespace tablator {
class Table;

// JTODO Class? Namespace?  It lacks state but does have private functions.

class Ipac_Table_Writer {
public:
    /*******************************************/
    /* Table-writer functions */
    /*******************************************/

    static void write_ipac_table(const Table &table, std::ostream &os);
    static void write_ipac_table_header(const Table &table, std::ostream &os,
                                        int num_rows = WRITE_ALL_ROWS);
    static void write_ipac_column_headers(const Table &table, std::ostream &os);
    static void write_ipac_subtable_by_row(const Table &table, std::ostream &os,
                                           std::vector<size_t> requested_row_ids);
    static void write_ipac_subtable_by_row(const Table &table, std::ostream &os,
                                           size_t start_row, size_t row_count);
    static void write_single_ipac_record(const Table &table, std::ostream &os,
                                         size_t row_idx);
    static void write_consecutive_ipac_records(const Table &table, std::ostream &os,
                                               size_t start_row, size_t row_count);
    static void write_selected_ipac_records(
            const Table &table, std::ostream &os,
            std::vector<size_t> const &requested_row_ids);

    /*******************************************/
    /* Auxiliary functions exposed through Table */
    /*******************************************/

    static std::string to_ipac_string(const Data_Type &type);
    static std::vector<size_t> get_column_widths(const Table &table);


    /*******************************************/
    /* Little helpers */
    /*******************************************/

    // JTODO  These should live elsewhere.
    static void convert_newlines(std::string &input) {
        boost::replace_all(input, "\n", "  ");
    }

    static const std::string convert_newlines(const std::string &input) {
        std::string temp_str(input);
        convert_newlines(temp_str);
        return temp_str;
    }


private:
    static constexpr int WRITE_ALL_ROWS = -1;


    static void write_ipac_column_headers(
            const Table &table, std::ostream &os,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_ipac_subtable_by_row(
            const Table &table, std::ostream &os, std::vector<size_t> requested_row_ids,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_ipac_subtable_by_row(
            const Table &table, std::ostream &os, size_t start_row, size_t row_count,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_single_ipac_record(
            const Table &table, std::ostream &os, size_t row_offset,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_consecutive_ipac_records(
            const Table &table, std::ostream &os, size_t start_row, size_t row_count,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_selected_ipac_records(
            const Table &table, std::ostream &os,
            std::vector<size_t> const &requested_row_ids,
            const std::vector<Data_Type> &datatypes_for_writing);

    // This one has no public counterpart.
    static void write_single_ipac_record_by_offset(
            const Table &table, std::ostream &os, size_t row_offset,
            const std::vector<Data_Type> &datatypes_for_writing);
};

}  // namespace tablator
