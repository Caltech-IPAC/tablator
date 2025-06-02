#include <array>
#include <limits>
#include <vector>

#include "../../Table.hxx"
#include "../../to_string.hxx"

namespace {

// JTODO move these
static constexpr ushort COL_NAME = 0;
  //static constexpr ushort COL_TYPE = 1;
static constexpr ushort COL_UNIT = 2;
static constexpr ushort COL_NULL = 3;



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

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};
    create_types_from_ipac_headers(columns, offsets, ipac_columns, ipac_column_widths);

 // std::cout << "read_ipac_table(), after create_types()" << std::endl;
#if 0
	for (const auto &offset : offsets) {
	  std::cout << "  offset: " << offset << std::endl;
	  }
#endif

	size_t num_tablator_columns = ipac_columns[COL_NAME].size();
	size_t line_num_after_headers = current_line_num;

    std::vector<size_t> minimum_column_widths(num_tablator_columns, 1);
    std::vector<bool> dynamic_array_flags(num_tablator_columns, false);

    std::string line;
    std::getline(input_stream, line);
    Row row_string(tablator::get_row_size(offsets));

    std::vector<uint8_t> data;
    while (input_stream) {
        if (line.find_first_not_of(" \t") != std::string::npos) {
            row_string.fill_with_zeros();
            for (size_t col_idx = 1; col_idx < num_tablator_columns; ++col_idx) {
				const auto &column = columns[col_idx];
                if (line[ipac_column_offsets[col_idx - 1]] != ' ')
                    throw std::runtime_error(
                            "Non-space found at a delimiter location on line " +
                            std::to_string(current_line_num) + ", col_idx " +
                            std::to_string(ipac_column_offsets[col_idx - 1]) +
                            " between the fields '" + columns[col_idx - 1].get_name() +
                            "' and '" + column.get_name() +
                            "'.  Is a field not wide enough?");

                std::string element = line.substr(ipac_column_offsets[col_idx - 1] + 1,
                                                  ipac_column_widths[col_idx]);
                boost::algorithm::trim(element);
				if (column.get_type() == Data_Type::CHAR && element.size() != minimum_column_widths[col_idx]) {
				  // Do this only for CHAR columns?  If we get here, array really is dynamic.
				  dynamic_array_flags[col_idx] = true;
				}
				// std::cout << "col_idx: " << col_idx << ", before, min_width_sofar: " << minimum_column_widths[col_idx] << std::endl;
                minimum_column_widths[col_idx] =
                        std::max(minimum_column_widths[col_idx], element.size());
				// std::cout << "col_idx: " << col_idx << ", after, min_width_sofar: " << minimum_column_widths[col_idx] << std::endl;
                if ((!ipac_columns[COL_NULL][col_idx].empty() &&
                     element == ipac_columns[COL_NULL][col_idx]) ||
                    (ipac_columns[COL_NULL][col_idx].empty() &&
                     column.get_type() != Data_Type::CHAR &&
                     element.empty())) {
				  // std::cout << "before set_null() TODO" << std::endl;
                    row_string.set_null(column.get_type(),
                                        column.get_array_size(), col_idx,
                                        offsets[col_idx], offsets[col_idx + 1]);
                } else {
				  // std::cout << "read_ipac_table(), not null, dynamic_flag: " << column.get_dynamic_array_flag() << std::endl;
                    try {
					  insert_ascii_in_row(row_string, column.get_type(),
										  column.get_array_size(), col_idx,
                                            element, offsets[col_idx],
										  offsets[col_idx + 1], column.get_dynamic_array_flag());
                    } catch (std::exception &error) {
                        throw std::runtime_error(
                                "Invalid " + to_string(column.get_type()) +
                                " for field '" + column.get_name() +
                                "' in line " + std::to_string(current_line_num + 1) +
                                ".  Found '" + element + "'");
                    }
                }
            }
            std::size_t bad_char(line.find_first_not_of(
                    " \t\r", ipac_column_offsets[num_tablator_columns - 1]));
            if (bad_char != std::string::npos)
                throw std::runtime_error("Non-whitespace found at the end of line " +
                                         std::to_string(current_line_num) +
                                         ", col_idx " + std::to_string(bad_char) +
                                         ": '" + line.substr(bad_char) +
                                         "'.\n\t  Is the header not wide enough?");

            tablator::append_row(data, row_string);
        }
        ++current_line_num;
        std::getline(input_stream, line);
    }

	// Adjust dynamic_array_flags and offsets  if necessary.
	for (size_t col_idx = 1; col_idx < num_tablator_columns; ++col_idx) {
	 auto &column = columns[col_idx];
	  if (column.get_type() == Data_Type::CHAR && !dynamic_array_flags[col_idx]) {
		// std::cout << "col_idx: " << col_idx << ", unsetting dynamic_array_flag" << std::endl;
		column.set_dynamic_array_flag(false);
#if 1
		if (current_line_num > line_num_after_headers && offsets[col_idx] < sizeof(uint32_t)) {
		  throw std::runtime_error("Offset too small for column of type CHAR.");
		}
		offsets[col_idx] -= sizeof(uint32_t);
#endif
	  }

	}

    shrink_ipac_string_columns_to_fit(columns, offsets, data, minimum_column_widths);

    Table_Element table_element =
            Table_Element::Builder(columns, offsets, data).build();
    add_resource_element(Resource_Element::Builder(table_element)
                                 .add_labeled_properties(labeled_resource_properties)
                                 .build());
}
