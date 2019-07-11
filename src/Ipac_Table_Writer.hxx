#pragma once

#include <string>
#include <vector>

#include "Data_Type.hxx"

namespace tablator {
class Table;

// JTODO Class? Namespace?  It lacks state but does have private functions.

class Ipac_Table_Writer {
public:
    /*******************************************/
    /* Helper functions */
    /*******************************************/

    static std::string to_ipac_string(const Data_Type &type);
    static std::vector<size_t> get_column_widths(const Table &table);

    // JTODO  These should live elsewhere.
    static void convert_newlines(std::string &input);
    static const std::string convert_newlines(const std::string &input);


    /*******************************************/
    /* Table-writer functions */
    /*******************************************/

    static void write_ipac_table(const Table &table, std::ostream &os);
    static void write_ipac_table_header(const Table &table, std::ostream &os,
                                        int num_rows = WRITE_ALL_ROWS);

private:
    static constexpr int WRITE_ALL_ROWS = -1;


    static void write_ipac_table(const Table &table, std::ostream &os,
                                 const std::vector<Data_Type> &datatypes_for_writing);

    static void write_ipac_column_headers(
            const Table &table, std::ostream &os,
            const std::vector<Data_Type> &datatypes_for_writing);

    static void write_consecutive_ipac_records(
            const Table &table, std::ostream &os, size_t start_row,
            size_t num_requested, const std::vector<Data_Type> &datatypes_for_writing);

    static void write_single_ipac_record_by_offset(
            const Table &table, std::ostream &os, size_t row_offset,
            const std::vector<Data_Type> &datatypes_for_writing);
};

}  // namespace tablator
