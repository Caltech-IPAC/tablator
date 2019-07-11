#include "../Ipac_Table_Writer.hxx"

#include <boost/algorithm/string.hpp>

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"
#include "../write_type_as_ascii.hxx"


/**********************************************************/
/* Utility functions */
/**********************************************************/

void tablator::Ipac_Table_Writer::convert_newlines(std::string& input) {
    boost::replace_all(input, "\n", "  ");
}

const std::string tablator::Ipac_Table_Writer::convert_newlines(
        const std::string& input) {
    std::string temp_str(input);
    convert_newlines(temp_str);
    return temp_str;
}

namespace {
size_t get_effective_array_size(tablator::Data_Type active_datatype,
                                       size_t orig_size) {
    return (active_datatype == tablator::Data_Type::CHAR) ? 1 : orig_size;
}

size_t get_effective_array_size(
        const tablator::Table& table,
        const std::vector<tablator::Data_Type>& datatypes_for_writing, size_t col_id) {
    tablator::Data_Type active_datatype =
            tablator::Data_Type_Adjuster::get_datatype_for_writing(
                    table, datatypes_for_writing, col_id);
    return get_effective_array_size(active_datatype, table.columns[col_id].array_size);
}

};  // namespace


/**********************************************************/
/* private class member functions */
/**********************************************************/

void tablator::Ipac_Table_Writer::write_ipac_table(
        const tablator::Table& table, std::ostream& os,
        const std::vector<Data_Type>& datatypes_for_writing) {
    tablator::Ipac_Table_Writer::write_ipac_table_header(table, os);

    // Write column names, types, units, etc.
    write_ipac_column_headers(table, os, datatypes_for_writing);

    // Write column values
    write_consecutive_ipac_records(table, os, 0, table.num_rows(),
                                   datatypes_for_writing);
}

/**********************************************************/

void tablator::Ipac_Table_Writer::write_ipac_column_headers(
        const Table& table, std::ostream& os,
        const std::vector<Data_Type>& datatypes_for_writing) {
    const std::vector<size_t> ipac_column_widths = table.get_column_widths();
    size_t total_record_width = 0;

    // Write column names
    os << "|";
    os << std::right;
    const auto& columns = table.columns;
    auto width_iter = std::next(ipac_column_widths.begin());
    for (size_t i = 1; i < columns.size(); ++i, ++width_iter) {
        auto column = columns[i];
        if (get_effective_array_size(table, datatypes_for_writing, i) == 1) {
            total_record_width += (*width_iter + 1);
            os << std::setw(*width_iter) << column.name << "|";
        } else {
            for (size_t element = 0; element < column.array_size; ++element) {
                total_record_width += (*width_iter + 1);
                os << std::setw(*width_iter)
                   << (column.name + "_" + std::to_string(element)) << "|";
            }
        }
    }
    os << "\n|";


    // Write column types
    width_iter = std::next(ipac_column_widths.begin());
    for (size_t i = 1; i < columns.size(); ++i, ++width_iter) {
        auto column = columns[i];
        Data_Type active_datatype = Data_Type_Adjuster::get_datatype_for_writing(
                table, datatypes_for_writing, i);
        size_t effective_array_size =
                get_effective_array_size(active_datatype, column.array_size);

        for (size_t element = 0; element < effective_array_size; ++element) {
            os << std::setw(*width_iter)
               << tablator::Ipac_Table_Writer::to_ipac_string(active_datatype) << "|";
        }
    }

    os << "\n|";

    // Write column units
    width_iter = std::next(ipac_column_widths.begin());
    for (size_t i = 1; i < columns.size(); ++i, ++width_iter) {
        auto column = columns[i];
        size_t effective_array_size =
                get_effective_array_size(table, datatypes_for_writing, i);

        auto unit = column.field_properties.attributes.find("unit");
        std::string unit_str =
                (unit == column.field_properties.attributes.end()) ? " " : unit->second;

        for (size_t element = 0; element < effective_array_size; ++element) {
            os << std::setw(*width_iter) << unit_str << "|";
        }
    }
    os << "\n|";

    // Write column null property
    width_iter = std::next(ipac_column_widths.begin());
    for (size_t i = 1; i < columns.size(); ++i, ++width_iter) {
        auto column = columns[i];
        size_t effective_array_size =
                get_effective_array_size(table, datatypes_for_writing, i);

        auto null = column.field_properties.values.null;
        std::string null_str = (null.empty()) ? "null" : null;

        for (size_t element = 0; element < effective_array_size; ++element) {
            os << std::setw(*width_iter) << null_str << "|";
        }
    }
    os << "\n";
}

/**********************************************************/

// JTODO error if num_requested is too big?
void tablator::Ipac_Table_Writer::write_consecutive_ipac_records(
        const Table& table, std::ostream& os, size_t start_row, size_t num_requested,
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
    size_t end_row = std::min(num_table_rows, start_row + num_requested);
    size_t row_size = table.row_size();
    size_t curr_row_offset = start_row * row_size;
    for (size_t row = start_row; row < end_row; ++row) {
        write_single_ipac_record_by_offset(table, os, curr_row_offset,
                                           datatypes_for_writing);
        curr_row_offset += row_size;
    }
}

/**********************************************************/

// JTODO validate?  quietly exit?
// This function writes any column of array-size n as n columns of array-size 1.
void tablator::Ipac_Table_Writer::write_single_ipac_record_by_offset(
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

    static constexpr const char* DEFAULT_NULL_VALUE = "null";
    const std::vector<size_t> ipac_column_widths = table.get_column_widths();

    const std::vector<Column>& columns = table.columns;

    // Skip the null bitfield flag
    for (size_t i = 1; i < columns.size(); ++i) {
        auto column = columns[i];
        Data_Type active_datatype = Data_Type_Adjuster::get_datatype_for_writing(
                table, datatypes_for_writing, i);

        if (table.is_null(curr_row_offset, i)) {
            auto null_value = column.field_properties.values.null;

            size_t effective_array_size =
                    get_effective_array_size(active_datatype, column.array_size);

            for (size_t element = 0; element < effective_array_size; ++element) {
                os << IPAC_COLUMN_SEPARATOR << std::setw(ipac_column_widths[i]);
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
                os << IPAC_COLUMN_SEPARATOR << std::setw(ipac_column_widths[i]);
                os << static_cast<const uint16_t>(*curr_data);
                curr_data += element_size;
            }
        } else {
            size_t base_offset = curr_row_offset + table.offsets[i];
            uint8_t const* curr_data = table.data.data() + base_offset;

            os << IPAC_COLUMN_SEPARATOR << std::setw(ipac_column_widths[i]);
            std::stringstream ss_temp;
            write_type_as_ascii(ss_temp, active_datatype, column.array_size, curr_data,
                                ipac_column_widths[i]);
            std::string s(ss_temp.str());
            convert_newlines(s);
            os << s;
        }
    }
    os << " \n";
}
