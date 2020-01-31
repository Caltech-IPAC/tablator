#include <array>
#include <limits>
#include <vector>

#include "../../Table.hxx"
#include "../../Utils/Table_Utils/insert_ascii_in_row.hxx"
#include "../../to_string.hxx"

namespace {
std::vector<size_t> get_ipac_column_widths(
        const std::vector<size_t> &ipac_column_offsets) {
    const size_t num_columns = ipac_column_offsets.size() - 1;
    std::vector<size_t> ipac_column_widths;
    /// Add a column for null flags.
    ipac_column_widths.push_back((num_columns + 7) / 8);
    for (size_t i = 0; i < num_columns; ++i)
        ipac_column_widths.push_back(ipac_column_offsets[i + 1] -
                                     ipac_column_offsets[i] - 1);
    return ipac_column_widths;
}
}  // namespace


void tablator::Table::read_ipac_table(std::istream &input_stream) {
    std::array<std::vector<std::string>, 4> ipac_columns;
    std::vector<size_t> ipac_column_offsets;

    std::vector<std::pair<std::string, Property>> labeled_resource_properties;
    size_t current_line_num =
            read_ipac_header(input_stream, ipac_columns, ipac_column_offsets,
                             labeled_resource_properties);

    const auto ipac_column_widths = get_ipac_column_widths(ipac_column_offsets);

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};
    create_types_from_ipac_headers(columns, offsets, ipac_columns, ipac_column_widths);

    std::vector<size_t> minimum_column_widths(ipac_columns[0].size(), 1);
    std::string line;
    std::getline(input_stream, line);
    Row row_string(tablator::row_size(offsets));

    std::vector<uint8_t> data;
    while (input_stream) {
        if (line.find_first_not_of(" \t") != std::string::npos) {
            row_string.set_zero();
            for (size_t column = 1; column < ipac_columns[0].size(); ++column) {
                if (line[ipac_column_offsets[column - 1]] != ' ')
                    throw std::runtime_error(
                            "Non-space found at a delimiter location on line " +
                            std::to_string(current_line_num) + ", column " +
                            std::to_string(ipac_column_offsets[column - 1]) +
                            " between the fields '" + columns[column - 1].get_name() +
                            "' and '" + columns[column].get_name() +
                            "'.  Is a field not wide enough?");

                std::string element = line.substr(ipac_column_offsets[column - 1] + 1,
                                                  ipac_column_widths[column]);
                boost::algorithm::trim(element);
                minimum_column_widths[column] =
                        std::max(minimum_column_widths[column], element.size());
                if ((!ipac_columns[3][column].empty() &&
                     element == ipac_columns[3][column]) ||
                    (ipac_columns[3][column].empty() &&
                     columns[column].get_type() != Data_Type::CHAR &&
                     element.empty())) {
                    row_string.set_null(columns[column].get_type(),
                                        columns[column].get_array_size(), column,
                                        offsets[column], offsets[column + 1]);
                } else {
                    try {
                        insert_ascii_in_row(columns[column].get_type(),
                                            columns[column].get_array_size(), column,
                                            element, offsets[column],
                                            offsets[column + 1], row_string);
                    } catch (std::exception &error) {
                        throw std::runtime_error(
                                "Invalid " + to_string(columns[column].get_type()) +
                                " for field '" + columns[column].get_name() +
                                "' in line " + std::to_string(current_line_num + 1) +
                                ".  Found '" + element + "'");
                    }
                }
            }
            std::size_t bad_char(line.find_first_not_of(
                    " \t\r", ipac_column_offsets[ipac_columns[0].size() - 1]));
            if (bad_char != std::string::npos)
                throw std::runtime_error("Non-whitespace found at the end of line " +
                                         std::to_string(current_line_num) +
                                         ", column " + std::to_string(bad_char) +
                                         ": '" + line.substr(bad_char) +
                                         "'.\n\t  Is the header not wide enough?");

            tablator::append_row(data, row_string);
        }
        ++current_line_num;
        std::getline(input_stream, line);
    }
    shrink_ipac_string_columns_to_fit(columns, offsets, data, minimum_column_widths);

    Table_Element table_element =
            Table_Element::Builder(columns, offsets, data).build();
    add_resource_element(Resource_Element::Builder(table_element)
                                 .add_labeled_properties(labeled_resource_properties)
                                 .build());
}
