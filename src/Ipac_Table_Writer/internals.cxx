#include <boost/range/algorithm/replace_copy_if.hpp>
#include <boost/range/algorithm/replace_if.hpp>

#include "../Ipac_Table_Writer.hxx"

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"
#include "../write_type_as_ascii.hxx"


// This file implements private functions of the Ipac_Table_Writer class.


namespace {

size_t get_effective_array_size(tablator::Data_Type active_datatype, size_t orig_size) {
    return (active_datatype == tablator::Data_Type::CHAR) ? 1 : orig_size;
}

const std::string compose_invalid_row_message(size_t row_id, size_t num_rows) {
    std::string msg = "invalid row_id ";
    msg.append(std::to_string(row_id))
            .append("; table has only ")
            .append(std::to_string(num_rows))
            .append(" rows.");
    return msg;
}

void validate_row_ids(const std::vector<size_t>& requested_row_ids,
                      size_t num_table_rows) {
    auto big_iter =
            std::find_if(requested_row_ids.begin(), requested_row_ids.end(),
                         [num_table_rows](size_t i) { return i >= num_table_rows; });
    if (big_iter != requested_row_ids.end()) {
        throw std::runtime_error(
                compose_invalid_row_message(*big_iter, num_table_rows));
    }
}

bool is_valid_col_id(size_t col_id, size_t num_columns) {
    return ((col_id > 0) && (col_id < num_columns));
}

};  // namespace


/**********************************************************/
/* Top-level functions that write tables */
/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& requested_row_ids,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    validate_row_ids(requested_row_ids, table.num_rows());

    // Write table-level header.
    tablator::Ipac_Table_Writer::write_header(table, os, requested_row_ids.size());

    // Write column names, types, units, etc.
    tablator::Ipac_Table_Writer::write_column_headers(table, os, ipac_column_widths,
                                                      datatypes_for_writing);

    // Write data.
    tablator::Ipac_Table_Writer::write_selected_records(
            table, os, requested_row_ids, ipac_column_widths, datatypes_for_writing);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(
        const Table& table, std::ostream& os, size_t start_row, size_t row_count,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();

    size_t true_row_count = 0;
    if (start_row < num_table_rows) {
        true_row_count = std::min(num_table_rows - start_row, row_count);
    }

    // Write table-level header.
    tablator::Ipac_Table_Writer::write_header(table, os, true_row_count);

    // Write column names, types, units, etc.
    tablator::Ipac_Table_Writer::write_column_headers(table, os, ipac_column_widths,
                                                      datatypes_for_writing);

    // Write data.
    tablator::Ipac_Table_Writer::write_consecutive_records(
            table, os, start_row, true_row_count, ipac_column_widths,
            datatypes_for_writing);
}

/**********************************************************/
/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& included_column_ids,
        const std::vector<size_t>& requested_row_ids,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    validate_row_ids(requested_row_ids, table.num_rows());

    // Write table-level header.
    tablator::Ipac_Table_Writer::write_header(table, os, requested_row_ids.size());

    // Write column names, types, units, etc.
    tablator::Ipac_Table_Writer::write_column_headers(
            table, os, included_column_ids, ipac_column_widths, datatypes_for_writing);

    // Write data.
    tablator::Ipac_Table_Writer::write_selected_records(
            table, os, included_column_ids, requested_row_ids, ipac_column_widths,
            datatypes_for_writing);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_column_and_row(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& included_column_ids, size_t start_row,
        size_t row_count, const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();

    size_t true_row_count = 0;
    if (start_row < num_table_rows) {
        true_row_count = std::min(num_table_rows - start_row, row_count);
    }

    // Write table-level header.
    tablator::Ipac_Table_Writer::write_header(table, os, true_row_count);
    // Write column names, types, units, etc.
    tablator::Ipac_Table_Writer::write_column_headers(
            table, os, included_column_ids, ipac_column_widths, datatypes_for_writing);

    // Write data.
    tablator::Ipac_Table_Writer::write_consecutive_records(
            table, os, included_column_ids, start_row, true_row_count,
            ipac_column_widths, datatypes_for_writing);
}


/**********************************************************/
/* Mid-level functions that extract and write records */
/**********************************************************/


// This function writes any column of array-size n as n columns of array-size 1.
void tablator::Ipac_Table_Writer::write_single_record(
        const Table& table, std::ostream& os, size_t row_id,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    if (row_id >= table.num_rows()) {
        return;
    }
    size_t curr_row_offset = row_id * table.row_size();
    write_single_record_by_offset(table, os, curr_row_offset, ipac_column_widths,
                                  datatypes_for_writing);
}

/**********************************************************/

// This function writes any column of array-size n as n columns of array-size 1.
void tablator::Ipac_Table_Writer::write_single_record(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& included_column_ids, size_t row_id,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    if (row_id >= table.num_rows()) {
        return;
    }
    size_t curr_row_offset = row_id * table.row_size();
    write_single_record_by_offset(table, os, included_column_ids, curr_row_offset,
                                  ipac_column_widths, datatypes_for_writing);
}

/**********************************************************/
/**********************************************************/

void tablator::Ipac_Table_Writer::write_consecutive_records(
        const Table& table, std::ostream& os, size_t start_row, size_t row_count,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();
    if (start_row >= num_table_rows) {
        return;
    }
    size_t end_row = start_row + std::min(num_table_rows - start_row, row_count);
    size_t row_size = table.row_size();
    size_t curr_row_offset = start_row * row_size;
    for (size_t row = start_row; row < end_row; ++row) {
        write_single_record_by_offset(table, os, curr_row_offset, ipac_column_widths,
                                      datatypes_for_writing);
        curr_row_offset += row_size;
    }
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_consecutive_records(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& included_column_ids, size_t start_row,
        size_t row_count, const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();
    if (start_row >= num_table_rows) {
        return;
    }
    size_t end_row = start_row + std::min(num_table_rows - start_row, row_count);
    size_t row_size = table.row_size();
    size_t curr_row_offset = start_row * row_size;
    for (size_t row = start_row; row < end_row; ++row) {
        write_single_record_by_offset(table, os, included_column_ids, curr_row_offset,
                                      ipac_column_widths, datatypes_for_writing);
        curr_row_offset += row_size;
    }
}

/**********************************************************/
/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_records(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& requested_row_ids,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();
    size_t row_size = table.row_size();
    for (size_t row : requested_row_ids) {
        if (row >= num_table_rows) {
            throw std::runtime_error(compose_invalid_row_message(row, num_table_rows));
        }
        write_single_record_by_offset(table, os, row * row_size, ipac_column_widths,
                                      datatypes_for_writing);
    }
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_records(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& included_column_ids,
        const std::vector<size_t>& requested_row_ids,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();
    size_t row_size = table.row_size();
    for (size_t row : requested_row_ids) {
        if (row >= num_table_rows) {
            throw std::runtime_error(compose_invalid_row_message(row, num_table_rows));
        }
        write_single_record_by_offset(table, os, included_column_ids, row * row_size,
                                      ipac_column_widths, datatypes_for_writing);
    }
}


/**********************************************************/
/* Mid-level functions that write column headers */
/**********************************************************/

// The IPAC table format does not support array-valued non-char columns.
// A array column of fixed size n named "col" of non-char type is formatted
// as single-valued columns named "col_0", "col_1", ... "col_{n-1}".
void tablator::Ipac_Table_Writer::write_column_headers(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    const auto& columns = table.columns;
    size_t total_record_width = 0;

    os << "|";
    os << std::right;

    // Write column names
    for (size_t col_id = 1; col_id < columns.size(); ++col_id) {
        size_t effective_array_size = get_effective_array_size(
                datatypes_for_writing[col_id], columns[col_id].array_size);
        total_record_width += write_column_name(
                table, os, col_id, ipac_column_widths[col_id], effective_array_size);
    }
    os << "\n|";

    // Write column types
    for (size_t col_id = 1; col_id < columns.size(); ++col_id) {
        write_column_type(table, os, datatypes_for_writing, col_id,
                          ipac_column_widths[col_id]);
    }

    os << "\n|";

    // Write column units
    for (size_t col_id = 1; col_id < columns.size(); ++col_id) {
        size_t effective_array_size = get_effective_array_size(
                datatypes_for_writing[col_id], columns[col_id].array_size);
        write_column_unit(table, os, col_id, ipac_column_widths[col_id],
                          effective_array_size);
    }
    os << "\n|";

    // Write column null property
    for (size_t col_id = 1; col_id < columns.size(); ++col_id) {
        size_t effective_array_size = get_effective_array_size(
                datatypes_for_writing[col_id], columns[col_id].array_size);
        write_column_null(table, os, col_id, ipac_column_widths[col_id],
                          effective_array_size);
    }
    os << "\n";
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_column_headers(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& included_column_ids,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    const auto& columns = table.columns;
    size_t total_record_width = 0;

    std::vector<size_t>::const_iterator cols_begin = included_column_ids.begin();

    os << "|";
    os << std::right;

    // Write column names

    for (std::vector<size_t>::const_iterator cols_iter = cols_begin;
         cols_iter != included_column_ids.end(); ++cols_iter) {
        auto col_id = *cols_iter;
        if (is_valid_col_id(col_id, columns.size())) {
            size_t effective_array_size = get_effective_array_size(
                    datatypes_for_writing[col_id], columns[col_id].array_size);
            total_record_width +=
                    write_column_name(table, os, col_id, ipac_column_widths[col_id],
                                      effective_array_size);
        }
    }
    os << "\n|";

    // Write column types
    for (std::vector<size_t>::const_iterator cols_iter = cols_begin;
         cols_iter != included_column_ids.end(); ++cols_iter) {
        auto col_id = *cols_iter;
        if (is_valid_col_id(col_id, columns.size())) {
            write_column_type(table, os, datatypes_for_writing, col_id,
                              ipac_column_widths[col_id]);
        }
    }

    os << "\n|";

    // Write column units
    for (std::vector<size_t>::const_iterator cols_iter = cols_begin;
         cols_iter != included_column_ids.end(); ++cols_iter) {
        auto col_id = *cols_iter;
        if (is_valid_col_id(col_id, columns.size())) {
            size_t effective_array_size = get_effective_array_size(
                    datatypes_for_writing[col_id], columns[col_id].array_size);
            write_column_unit(table, os, col_id, ipac_column_widths[col_id],
                              effective_array_size);
        }
    }
    os << "\n|";

    // Write column null property
    for (std::vector<size_t>::const_iterator cols_iter = cols_begin;
         cols_iter != included_column_ids.end(); ++cols_iter) {
        auto col_id = *cols_iter;
        if (is_valid_col_id(col_id, columns.size())) {
            size_t effective_array_size = get_effective_array_size(
                    datatypes_for_writing[col_id], columns[col_id].array_size);
            write_column_null(table, os, col_id, ipac_column_widths[col_id],
                              effective_array_size);
        }
    }
    os << "\n";
}


/**********************************************************/
/* Private functions that do the heavy lifting. */
/**********************************************************/

/**********************************************************/
/* The private helper functions write_column_XXX() below do not validate col_id. */
/**********************************************************/

size_t tablator::Ipac_Table_Writer::write_column_name(const Table& table,
                                                      std::ostream& os, size_t col_id,
                                                      size_t col_width,
                                                      size_t effective_array_size) {
    auto& column = table.columns[col_id];
    size_t total_width;

    std::size_t first = column.name.find_first_of("\n\r|");
    if (first != std::string::npos) {
        std::string msg("column name contains illegal character (\\n, \\r, or |): ");
        throw(std::runtime_error(msg + column.name));
    }

    if (effective_array_size == 1) {
        total_width = (col_width + 1);
        os << std::setw(col_width) << column.name << "|";
    } else {
        total_width = (col_width + 1) * column.array_size;
        for (size_t element = 0; element < column.array_size; ++element) {
            os << std::setw(col_width) << (column.name + "_" + std::to_string(element))
               << "|";
        }
    }
    return total_width;
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_column_type(
        const Table& table, std::ostream& os,
        const std::vector<Data_Type>& datatypes_for_writing, size_t col_id,
        size_t col_width) {
    auto& column = table.columns[col_id];
    Data_Type active_datatype = datatypes_for_writing[col_id];
    size_t effective_array_size =
            get_effective_array_size(active_datatype, column.array_size);

    for (size_t element = 0; element < effective_array_size; ++element) {
        os << std::setw(col_width)
           << tablator::Ipac_Table_Writer::to_ipac_string(active_datatype) << "|";
    }
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_column_unit(const Table& table,
                                                    std::ostream& os, size_t col_id,
                                                    size_t col_width,
                                                    size_t effective_array_size) {
    auto& column = table.columns[col_id];

    // Set default and adjust
    std::string unit_str = " ";
    auto unit = column.field_properties.attributes.find("unit");
    if (unit != column.field_properties.attributes.end()) {
        std::size_t first = unit->second.find_first_of("|");
        if (first != std::string::npos) {
            std::string msg("unit name contains illegal character '|': ");
            throw(std::runtime_error(msg + unit->second));
        }
        unit_str.clear();
        boost::replace_copy_if(unit->second, std::back_inserter(unit_str),
                               boost::is_any_of(NEWLINES), ' ');
    }

    for (size_t element = 0; element < effective_array_size; ++element) {
        os << std::setw(col_width) << unit_str << "|";
    }
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_column_null(const Table& table,
                                                    std::ostream& os, size_t col_id,
                                                    size_t col_width,
                                                    size_t effective_array_size) {
    auto& column = table.columns[col_id];
    auto& null_value = column.field_properties.values.null;
    const std::string& null_str =
            (null_value.empty()) ? tablator::Table::DEFAULT_NULL_VALUE : null_value;

    for (size_t element = 0; element < effective_array_size; ++element) {
        os << std::setw(col_width) << null_str << "|";
    }
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_value(
        const tablator::Table& table, std::ostream& os, size_t column_id,
        size_t curr_row_offset, size_t width,
        const std::vector<tablator::Data_Type>& datatypes_for_writing) {
    if (!is_valid_col_id(column_id, table.columns.size())) {
        // shouldn't happen; internal caller should have validated
        return;
    }
    size_t final_row_offset = table.data.size() - table.row_size();
    if (curr_row_offset > final_row_offset) {
        // shouldn't happen; internal caller should have validated
        return;
    }

    auto& column = table.columns[column_id];

    tablator::Data_Type active_datatype = datatypes_for_writing[column_id];
    if (table.is_null(curr_row_offset, column_id)) {
        auto& null_value = column.field_properties.values.null;
        const std::string& null_str =
                (null_value.empty()) ? tablator::Table::DEFAULT_NULL_VALUE : null_value;
        size_t effective_array_size =
                get_effective_array_size(active_datatype, column.array_size);

        for (size_t element = 0; element < effective_array_size; ++element) {
            os << IPAC_COLUMN_SEPARATOR << std::setw(width);
            os << null_str;
        }
    } else if (active_datatype == tablator::Data_Type::UINT8_LE) {
        // Do this case manually because write_type_as_ascii()
        // isn't equipped to write bytes as ints, as IPAC_FORMAT
        // requires.
        size_t base_offset = curr_row_offset + table.offsets[column_id];
        uint8_t const* curr_data = table.data.data() + base_offset;
        size_t element_size = data_size(active_datatype);

        for (size_t element = 0; element < column.array_size; ++element) {
            os << IPAC_COLUMN_SEPARATOR << std::setw(width);
            os << static_cast<const uint16_t>(*curr_data);
            curr_data += element_size;
        }
    } else {
        size_t base_offset = curr_row_offset + table.offsets[column_id];
        uint8_t const* curr_data = table.data.data() + base_offset;

        os << IPAC_COLUMN_SEPARATOR << std::setw(width);
        std::stringstream ss_temp;
        write_type_as_ascii(ss_temp, column.type, column.array_size, curr_data, width);
        std::string s;
        boost::replace_copy_if(ss_temp.str(), std::back_inserter(s),
                               boost::is_any_of(NEWLINES), ' ');
        os << s;
    }
}

/**********************************************************/
/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_record_by_offset(
        const Table& table, std::ostream& os, size_t curr_row_offset,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t final_row_offset = table.data.size() - table.row_size();
    if (curr_row_offset > final_row_offset) {
        // shouldn't happen; internal caller should have validated
        return;
    }

    const std::vector<Column>& columns = table.columns;

    // Skip the null bitfield flag
    for (size_t col_id = 1; col_id < columns.size(); ++col_id) {
        write_single_value(table, os, col_id, curr_row_offset,
                           ipac_column_widths[col_id], datatypes_for_writing);
    }
    os << IPAC_COLUMN_SEPARATOR << "\n";
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_record_by_offset(
        const Table& table, std::ostream& os,
        const std::vector<size_t>& included_column_ids, size_t curr_row_offset,
        const std::vector<size_t>& ipac_column_widths,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t final_row_offset = table.data.size() - table.row_size();
    if (curr_row_offset > final_row_offset) {
        // shouldn't happen; internal caller should have validated
        return;
    }

    const std::vector<Column>& columns = table.columns;

    // Skip the null bitfield flag
    std::vector<size_t>::const_iterator cols_iter = included_column_ids.begin();
    for (/* */; cols_iter != included_column_ids.end(); ++cols_iter) {
        auto col_id = *cols_iter;
        if (is_valid_col_id(col_id, columns.size())) {
            write_single_value(table, os, col_id, curr_row_offset,
                               ipac_column_widths[col_id], datatypes_for_writing);
        }
    }
    os << IPAC_COLUMN_SEPARATOR << "\n";
}
