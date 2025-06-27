#include "../../Table.hxx"

#include <array>
#include <limits>
#include <vector>


#include "../../to_string.hxx"
#include "../read_ipac_table.hxx"

// As of 28Jun25, all columns are created with dynamic_array_flag value <false>.

namespace {

std::vector<size_t> get_ipac_column_widths(
        const std::vector<size_t> &ipac_column_offsets) {
    const size_t num_columns = ipac_column_offsets.size() - 1;
    std::vector<size_t> ipac_column_widths;
    // Add a column for null flags.
    ipac_column_widths.push_back(tablator::bits_to_bytes(num_columns));
    for (size_t i = 0; i < num_columns; ++i)
        ipac_column_widths.push_back(ipac_column_offsets[i + 1] -
                                     ipac_column_offsets[i] - 1);
    return ipac_column_widths;
}
}  // namespace


void tablator::Table::read_ipac_table(std::istream &input_stream) {
    std::array<std::vector<std::string>, 4> ipac_columns;
    std::vector<size_t> ipac_column_offsets;

    Labeled_Properties labeled_resource_properties;
    size_t current_line_num =
            read_ipac_header(input_stream, ipac_columns, ipac_column_offsets,
                             labeled_resource_properties);

    const auto ipac_column_widths = get_ipac_column_widths(ipac_column_offsets);

    Field_Framework field_framework =
            create_types_from_ipac_headers(ipac_columns, ipac_column_widths);

    size_t num_tab_columns = ipac_columns[COL_NAME_IDX].size();
    std::vector<Column> &tab_columns = field_framework.get_columns();
    std::vector<size_t> &offsets = field_framework.get_offsets();

    std::vector<size_t> minimum_column_widths(num_tab_columns, 1);
    std::string line;
    std::getline(input_stream, line);

    std::vector<uint8_t> data;
    // JTODO reserve()

    Row single_row(field_framework.get_row_size());
    while (input_stream) {
        if (line.find_first_not_of(" \t") != std::string::npos) {
            single_row.fill_with_zeros();
            for (size_t col_idx = 1; col_idx < num_tab_columns; ++col_idx) {
                const auto &tab_column = tab_columns[col_idx];
                if (line[ipac_column_offsets[col_idx - 1]] != ' ')
                    throw std::runtime_error(
                            "Non-space found at a delimiter location on line " +
                            std::to_string(current_line_num) + ", col_idx " +
                            std::to_string(ipac_column_offsets[col_idx - 1]) +
                            " between the fields '" +
                            tab_columns[col_idx - 1].get_name() + "' and '" +
                            tab_column.get_name() + "'.  Is a field not wide enough?");

                std::string element = line.substr(ipac_column_offsets[col_idx - 1] + 1,
                                                  ipac_column_widths[col_idx]);
                boost::algorithm::trim(element);
                minimum_column_widths[col_idx] =
                        std::max(minimum_column_widths[col_idx], element.size());

                if ((!ipac_columns[COL_NULL_IDX][col_idx].empty() &&
                     element == ipac_columns[COL_NULL_IDX][col_idx]) ||
                    (ipac_columns[COL_NULL_IDX][col_idx].empty() &&
                     tab_column.get_type() != Data_Type::CHAR && element.empty())) {
                    single_row.insert_null(tab_column.get_type(),
                                           tab_column.get_array_size(), col_idx,
                                           offsets[col_idx], offsets[col_idx + 1]);
                } else {
                    try {
                        single_row.insert_from_ascii(element, tab_column.get_type(),
                                                     tab_column.get_array_size(),
                                                     col_idx, offsets[col_idx],
                                                     offsets[col_idx + 1]);
                    } catch (std::exception &error) {
                        throw std::runtime_error(
                                "Invalid " + to_string(tab_column.get_type()) +
                                " for field '" + tab_column.get_name() + "' in line " +
                                std::to_string(current_line_num + 1) + ".  Found '" +
                                element + "'");
                    }
                }
            }
            std::size_t bad_char(line.find_first_not_of(
                    " \t\r", ipac_column_offsets[num_tab_columns - 1]));
            if (bad_char != std::string::npos)
                throw std::runtime_error("Non-whitespace found at the end of line " +
                                         std::to_string(current_line_num) +
                                         ", col_idx " + std::to_string(bad_char) +
                                         ": '" + line.substr(bad_char) +
                                         "'.\n\t  Is the header not wide enough?");

            tablator::append_row(data, single_row);
        }
        ++current_line_num;
        std::getline(input_stream, line);
    }
    shrink_ipac_string_columns_to_fit(field_framework, data, minimum_column_widths);

    Table_Element table_element = Table_Element::Builder(field_framework, data).build();
    add_resource_element(Resource_Element::Builder(table_element)
                                 .add_labeled_properties(labeled_resource_properties)
                                 .build());
}
