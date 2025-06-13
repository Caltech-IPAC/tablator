#include "../../Table.hxx"

#include <array>
#include <limits>
#include <vector>


#include "../../to_string.hxx"
#include "../read_ipac_table.hxx"

namespace {

std::vector<size_t> get_incoming_column_widths(
        const std::vector<size_t> &incoming_column_offsets) {
    const size_t num_columns = incoming_column_offsets.size() - 1;
    std::vector<size_t> incoming_column_widths;
    // Add a column for null flags.
    incoming_column_widths.push_back(tablator::bits_to_bytes(num_columns));

    for (size_t i = 0; i < num_columns; ++i)
        incoming_column_widths.push_back(incoming_column_offsets[i + 1] -
                                     incoming_column_offsets[i] - 1);
    return incoming_column_widths;
}
}  // namespace


void tablator::Table::read_ipac_table(std::istream &input_stream) {
    std::array<std::vector<std::string>, 4> incoming_columns;
    std::vector<size_t> incoming_column_offsets;

    Labeled_Properties labeled_resource_properties;
    size_t line_num_after_headers = 
            read_ipac_header(input_stream, incoming_columns, incoming_column_offsets,
                             labeled_resource_properties);
	size_t current_line_num = line_num_after_headers;

	auto pos_after_headers = input_stream.tellg();
    const auto incoming_column_widths = get_incoming_column_widths(incoming_column_offsets);

    std::vector<Column> tab_columns;
    std::vector<size_t> offsets = {0};
    create_types_from_ipac_headers(tab_columns, offsets, incoming_columns,
                                   incoming_column_widths);

    size_t num_tablator_columns = incoming_columns[COL_NAME_IDX].size();
	//	size_t line_num_after_headers = current_line_num;

    std::vector<size_t> max_column_widths_sofar(num_tablator_columns, 1);


	// As we iterate through the rows, we'll set the
	// dynamic_array_flag value to TRUE for columns of type CHAR whose
	// per-row array_size values are not always the maximum.
	// (create_types_from_ipac_headers() sets the flag values to FALSE
	// by default.)   We'll adjust offsets accordingly in
	// shrink_ipac_columns_to_fit() immediately following this loop.

	// JTODO or just assume not dynamic?

	//	Non-char columns of tables in IPAC table format must have fixed length. 

	// Make a preliminary pass through data to set dynamic_array_flag
	// value for CHAR columns and to determine the minimum possible
	// width of each column.


    std::string line;
    std::getline(input_stream, line);
    while (input_stream) {
        if (line.find_first_not_of(" \t") != std::string::npos) {
            for (size_t col_idx = 1; col_idx < num_tablator_columns; ++col_idx) {
			  auto &tab_column = tab_columns[col_idx];
                if (line[incoming_column_offsets[col_idx - 1]] != ' ')
                    throw std::runtime_error(
                            "Non-space found at a delimiter location on line " +
                            std::to_string(current_line_num) + ", col_idx " +
                            std::to_string(incoming_column_offsets[col_idx - 1]) +
                            " between the fields '" +
                            tab_columns[col_idx - 1].get_name() + "' and '" +
                            tab_column.get_name() + "'.  Is a field not wide enough?");

                std::string element = line.substr(incoming_column_offsets[col_idx - 1] + 1,
                                                  incoming_column_widths[col_idx]);
                boost::algorithm::trim(element);
				if (tab_column.get_type() == Data_Type::CHAR && element.size() != max_column_widths_sofar[col_idx]) {
				  tab_column.set_dynamic_array_flag(true);
				}
                max_column_widths_sofar[col_idx] =
                        std::max(max_column_widths_sofar[col_idx], element.size());
            }
            std::size_t bad_char(line.find_first_not_of(
                    " \t\r", incoming_column_offsets[num_tablator_columns - 1]));
            if (bad_char != std::string::npos)
                throw std::runtime_error("Non-whitespace found at the end of line " +
                                         std::to_string(current_line_num) +
                                         ", col_idx " + std::to_string(bad_char) +
                                         ": '" + line.substr(bad_char) +
                                         "'.\n\t  Is the header not wide enough?");
        }
        ++current_line_num;
        std::getline(input_stream, line);
    }

    shrink_ipac_string_columns_to_fit(tab_columns, offsets, 
                                      max_column_widths_sofar);


	// Load data
    Row row_string(tablator::get_row_size(offsets));
    std::vector<uint8_t> data;

	// reset

	input_stream.clear();
	input_stream.seekg(pos_after_headers, std::ios_base::beg);

	current_line_num = line_num_after_headers;

    while (input_stream) {
        if (line.find_first_not_of(" \t") != std::string::npos) {
            row_string.fill_with_zeros();

            for (size_t col_idx = 1; col_idx < num_tablator_columns; ++col_idx) {
                const auto &tab_column = tab_columns[col_idx];

                std::string element = line.substr(incoming_column_offsets[col_idx - 1] + 1,
                                                  incoming_column_widths[col_idx]);
                boost::algorithm::trim(element);

                if ((!incoming_columns[COL_NULL_IDX][col_idx].empty() &&
                     element == incoming_columns[COL_NULL_IDX][col_idx]) ||
                    (incoming_columns[COL_NULL_IDX][col_idx].empty() &&
                     tab_column.get_type() != Data_Type::CHAR && element.empty())) {

                    row_string.set_null(tab_column.get_type(),
                                        tab_column.get_array_size(), col_idx,
                                        offsets[col_idx], offsets[col_idx + 1], tab_column.get_dynamic_array_flag());
                } else {
                    try {
                        insert_ascii_in_row(row_string, tab_column.get_type(),
                                            tab_column.get_array_size(), col_idx,
                                            element, offsets[col_idx],
                                            offsets[col_idx + 1], tab_column.get_dynamic_array_flag());
                    } catch (std::exception &error) {
                        throw std::runtime_error(
                                "Invalid " + to_string(tab_column.get_type()) +
                                " for field '" + tab_column.get_name() + "' in line " +
                                std::to_string(current_line_num + 1) + ".  Found '" +
                                element + "'");
                    }
                }
            }
            tablator::append_row(data, row_string);
        }
        ++current_line_num;
        std::getline(input_stream, line);
    }
    Table_Element table_element =
            Table_Element::Builder(tab_columns, offsets, data).build();
    add_resource_element(Resource_Element::Builder(table_element)
                                 .add_labeled_properties(labeled_resource_properties)
                                 .build());
}
