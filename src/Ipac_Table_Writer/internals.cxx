#include "../Ipac_Table_Writer.hxx"

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"
#include "../write_type_as_ascii.hxx"


// This file implements private functions of the Ipac_Table_Writer class.


namespace {

size_t get_effective_array_size(tablator::Data_Type active_datatype, size_t orig_size) {
    return (active_datatype == tablator::Data_Type::CHAR) ? 1 : orig_size;
}

};  // namespace


/**********************************************************/
/* Top-level functions that write tables */
/**********************************************************/

static constexpr const char* DEFAULT_NULL_VALUE = "null";


/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(
        const Table& table, std::ostream& os, std::vector<size_t> requested_row_ids,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();
    size_t num_valid_row_ids =
            std::count_if(requested_row_ids.begin(), requested_row_ids.end(),
                          [num_table_rows](size_t i) { return i < num_table_rows; });

    // Write table-level header.
    tablator::Ipac_Table_Writer::write_header(table, os, num_valid_row_ids);

    // Write column names, types, units, etc.
    tablator::Ipac_Table_Writer::write_column_headers(table, os, datatypes_for_writing);

    // Write data.
    tablator::Ipac_Table_Writer::write_selected_records(table, os, requested_row_ids,
                                                        datatypes_for_writing);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_subtable_by_row(
        const Table& table, std::ostream& os, size_t start_row, size_t row_count,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t true_row_count = std::min(table.num_rows() - start_row, row_count);

    // Write table-level header.
    tablator::Ipac_Table_Writer::write_header(table, os, true_row_count);

    // Write column names, types, units, etc.
    tablator::Ipac_Table_Writer::write_column_headers(table, os, datatypes_for_writing);

    // Write data.
    tablator::Ipac_Table_Writer::write_consecutive_records(
            table, os, start_row, true_row_count, datatypes_for_writing);
}

/**********************************************************/
/* Mid-level functions that extract and write records */
/**********************************************************/

// JTODO error if row_count is too big?
void tablator::Ipac_Table_Writer::write_consecutive_records(
        const Table& table, std::ostream& os, size_t start_row, size_t row_count,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();
    if (start_row > num_table_rows) {
        std::string msg = "Invalid start_row ";
        msg.append(std::to_string(start_row))
                .append("; only")
                .append(std::to_string(num_table_rows))
                .append(" rows in table.");
        throw std::runtime_error(msg);
    }
    size_t end_row = std::min(num_table_rows, start_row + row_count);
    size_t row_size = table.row_size();
    size_t curr_row_offset = start_row * row_size;
    for (size_t row = start_row; row < end_row; ++row) {
        write_single_record_by_offset(table, os, curr_row_offset,
                                      datatypes_for_writing);
        curr_row_offset += row_size;
    }
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_selected_records(
        const Table& table, std::ostream& os,
        std::vector<size_t> const& requested_row_ids,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t num_table_rows = table.num_rows();
    size_t row_size = table.row_size();
    for (size_t row : requested_row_ids) {
        if (row < num_table_rows) {
            write_single_record_by_offset(table, os, row * row_size,
                                          datatypes_for_writing);
        }
    }
}

/**********************************************************/

// This function writes any column of array-size n as n columns of array-size 1.
void tablator::Ipac_Table_Writer::write_single_record(
        const Table& table, std::ostream& os, size_t row_id,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t curr_row_offset = row_id * table.row_size();
    write_single_record_by_offset(table, os, curr_row_offset, datatypes_for_writing);
}


/**********************************************************/
/* Functions that do the work */
/**********************************************************/

// The IPAC table format does not support array-valued non-char columns.
// A array column of fixed size n named "col" of non-char type is formatted
// as single-valued columns named "col_0", "col_1", ... "col_{n-1}".

void tablator::Ipac_Table_Writer::write_column_headers(
        const Table& table, std::ostream& os,
        const std::vector<Data_Type>& datatypes_for_writing) {
    const std::vector<size_t> ipac_column_widths = table.get_column_widths();
    size_t total_record_width = 0;

    // Write column names
    os << "|";
    os << std::right;
    const auto& columns = table.columns;
    for (size_t i = 1; i < columns.size(); ++i) {
        auto column = columns[i];
        auto width = ipac_column_widths[i];
        if (get_effective_array_size(datatypes_for_writing[i], column.array_size) ==
            1) {
            total_record_width += (width + 1);
            os << std::setw(width) << column.name << "|";
        } else {
            for (size_t element = 0; element < column.array_size; ++element) {
                total_record_width += (width + 1);
                os << std::setw(width) << (column.name + "_" + std::to_string(element))
                   << "|";
            }
        }
    }
    os << "\n|";


    // Write column types
    for (size_t i = 1; i < columns.size(); ++i) {
        auto column = columns[i];
        Data_Type active_datatype = datatypes_for_writing[i];
        size_t effective_array_size =
                get_effective_array_size(active_datatype, column.array_size);

        for (size_t element = 0; element < effective_array_size; ++element) {
            os << std::setw(ipac_column_widths[i])
               << tablator::Ipac_Table_Writer::to_ipac_string(active_datatype) << "|";
        }
    }

    os << "\n|";

    // Write column units
    for (size_t i = 1; i < columns.size(); ++i) {
        auto column = columns[i];
        size_t effective_array_size =
                get_effective_array_size(datatypes_for_writing[i], column.array_size);
        auto unit = column.field_properties.attributes.find("unit");
        std::string unit_str =
                (unit == column.field_properties.attributes.end()) ? " " : unit->second;

        for (size_t element = 0; element < effective_array_size; ++element) {
            os << std::setw(ipac_column_widths[i]) << unit_str << "|";
        }
    }
    os << "\n|";

    // Write column null property
    for (size_t i = 1; i < columns.size(); ++i) {
        auto column = columns[i];
        size_t effective_array_size =
                get_effective_array_size(datatypes_for_writing[i], column.array_size);
        auto null = column.field_properties.values.null;
        std::string null_str = (null.empty()) ? DEFAULT_NULL_VALUE : null;

        for (size_t element = 0; element < effective_array_size; ++element) {
            os << std::setw(ipac_column_widths[i]) << null_str << "|";
        }
    }
    os << "\n";
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_single_record_by_offset(
        const Table& table, std::ostream& os, size_t curr_row_offset,
        const std::vector<Data_Type>& datatypes_for_writing) {
    size_t final_row_offset = table.data.size() - table.row_size();
    if (curr_row_offset > final_row_offset) {
        std::string msg = "invalid row_offset ";
        msg.append(std::to_string(curr_row_offset))
                .append("; largest possible is ")
                .append(std::to_string(final_row_offset))
                .append(".");
        throw std::runtime_error(msg);
    }

    const std::vector<size_t> ipac_column_widths = get_column_widths(table);

    const std::vector<Column>& columns = table.columns;

    // Skip the null bitfield flag
    for (size_t i = 1; i < columns.size(); ++i) {
        auto column = columns[i];
        auto width = ipac_column_widths[i];
        Data_Type active_datatype = datatypes_for_writing[i];
        if (table.is_null(curr_row_offset, i)) {
            auto null_value = column.field_properties.values.null;

            size_t effective_array_size =
                    get_effective_array_size(active_datatype, column.array_size);

            for (size_t element = 0; element < effective_array_size; ++element) {
                os << IPAC_COLUMN_SEPARATOR << std::setw(width);
                os << (null_value.empty() ? DEFAULT_NULL_VALUE : null_value);
            }
        } else if (active_datatype == tablator::Data_Type::UINT8_LE) {
            // Do this case manually because write_type_as_ascii()
            // isn't equipped to write bytes as ints, as IPAC_FORMAT
            // requires.
            size_t base_offset = curr_row_offset + table.offsets[i];
            uint8_t const* curr_data = table.data.data() + base_offset;
            size_t element_size = data_size(active_datatype);

            for (size_t element = 0; element < column.array_size; ++element) {
                os << IPAC_COLUMN_SEPARATOR << std::setw(width);
                os << static_cast<const uint16_t>(*curr_data);
                curr_data += element_size;
            }
        } else {
            size_t base_offset = curr_row_offset + table.offsets[i];
            uint8_t const* curr_data = table.data.data() + base_offset;

            os << IPAC_COLUMN_SEPARATOR << std::setw(width);
            std::stringstream ss_temp;
            write_type_as_ascii(ss_temp, active_datatype, column.array_size, curr_data,
                                width);
            std::string s(ss_temp.str());
            os << boost::replace_all_copy(s, "\n", " ");
        }
    }
    os << " \n";
}
