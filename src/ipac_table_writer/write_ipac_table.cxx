#include <inttypes.h>
#include <cmath>
#include <iomanip>
#include <limits>

#include "../Data_Type_Adjuster.hxx"
#include "../Table.hxx"
#include "../write_type_as_ascii.hxx"


/**********************************************************/
/* helper function */
/**********************************************************/

size_t get_effective_array_size(tablator::Data_Type active_datatype, size_t orig_size) {
    return (active_datatype == tablator::Data_Type::CHAR) ? 1 : orig_size;
}

size_t get_effective_array_size(
        const tablator::Table &table,
        const std::vector<tablator::Data_Type> &datatypes_for_writing, size_t col_id) {
    tablator::Data_Type active_datatype =
            tablator::Data_Type_Adjuster::get_datatype_for_writing(
                    table, datatypes_for_writing, col_id);
    return get_effective_array_size(active_datatype, table.columns[col_id].array_size);
}

/**********************************************************/
/* class member functions */
/**********************************************************/


void tablator::Table::write_ipac_table(std::ostream &os) const {
    write_ipac_table(os, Data_Type_Adjuster(*this).get_datatypes_for_writing(
                                 Format::Enums::IPAC_TABLE));
}

/**********************************************************/

void tablator::Table::write_ipac_table(
        std::ostream &os, const std::vector<Data_Type> &datatypes_for_writing) const {
    std::vector<size_t> ipac_column_widths = get_column_widths();
    write_ipac_table_header(os);

    // non-CHAR arrays are handled by a hack for whose benefit we pretend
    // that arrays being written as CHAR have size 1.
    int total_record_width = 0;

    // Write column names
    os << "|";
    os << std::right;
    auto width_iter = std::next(ipac_column_widths.begin());
    for (size_t i = 1; i < columns.size(); ++i, ++width_iter) {
        auto &column = columns[i];
        if (get_effective_array_size(*this, datatypes_for_writing, i) == 1) {
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
        auto &column = columns[i];
        Data_Type active_datatype = Data_Type_Adjuster::get_datatype_for_writing(
                *this, datatypes_for_writing, i);

        for (size_t element = 0;
             element < get_effective_array_size(active_datatype, column.array_size);
             ++element) {
            os << std::setw(*width_iter) << to_ipac_string(active_datatype) << "|";
        }
    }

    os << "\n|";

    // Write column units
    width_iter = std::next(ipac_column_widths.begin());
    for (size_t i = 1; i < columns.size(); ++i, ++width_iter) {
        auto &column = columns[i];

        auto unit = column.field_properties.attributes.find("unit");
        std::string unit_str =
                (unit == column.field_properties.attributes.end()) ? " " : unit->second;

        for (size_t element = 0;
             element < get_effective_array_size(*this, datatypes_for_writing, i);
             ++element) {
            os << std::setw(*width_iter) << unit_str << "|";
        }
    }
    os << "\n|";

    // Write column null property
    width_iter = std::next(ipac_column_widths.begin());
    for (size_t i = 1; i < columns.size(); ++i, ++width_iter) {
        auto &column = columns[i];

        auto null = column.field_properties.values.null;
        std::string null_str = (null.empty()) ? "null" : null;

        for (size_t element = 0;
             element < get_effective_array_size(*this, datatypes_for_writing, i);
             ++element) {
            os << std::setw(*width_iter) << null_str << "|";
        }
    }
    os << "\n";

    // Write column value
    std::stringstream ss;
    for (size_t row_offset = 0; row_offset < data.size(); row_offset += row_size()) {
        /// Skip the null bitfield flag
        for (size_t i = 1; i < columns.size(); ++i) {
            auto &column = columns[i];

            Data_Type active_datatype = Data_Type_Adjuster::get_datatype_for_writing(
                    *this, datatypes_for_writing, i);

            for (size_t element = 0;
                 element < get_effective_array_size(active_datatype, column.array_size);
                 ++element) {
                ss << " " << std::setw(ipac_column_widths[i]);
                size_t offset =
                        element * data_size(active_datatype) + offsets[i] + row_offset;

                if (is_null(row_offset, i)) {
                    auto null_value = column.field_properties.values.null;
                    if (null_value.empty())
                        ss << "null";
                    else
                        ss << null_value;
                } else {
                    /// Do some gymnastics because we want to write a byte
                    /// as an int
                    if (active_datatype == Data_Type::UINT8_LE) {
                        ss << static_cast<const uint16_t>(
                                static_cast<uint8_t>(*(data.data() + offset)));
                    } else {
                        std::stringstream ss_temp;
                        write_type_as_ascii(
                                ss_temp, column.type,
                                (active_datatype == Data_Type::CHAR ? column.array_size
                                                                    : 1),
                                data.data() + offset, ' ');
                        std::string s(ss_temp.str());
                        /// Turn newlines into spaces
                        auto newline_location(s.find('\n'));
                        while (newline_location != std::string::npos) {
                            s[newline_location] = ' ';
                            newline_location = s.find('\n', newline_location + 1);
                        }
                        ss << s;
                    }
                }
            }
        }
        ss << " \n";
        os << ss.str();
        ss.str("");
    }
}
