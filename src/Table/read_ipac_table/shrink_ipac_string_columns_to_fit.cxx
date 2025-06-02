#include "../../Table.hxx"

#include <stdexcept>
#include <utility>

void tablator::Table::shrink_ipac_string_columns_to_fit(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        std::vector<uint8_t> &data, const std::vector<size_t> &minimum_column_data_widths) {
    std::vector<size_t> new_offsets = {0};
    std::vector<Column> new_columns(columns);

    size_t old_row_size(tablator::get_row_size(offsets));
    size_t new_row_size(0);
	// std::cout << "shrink(), old_row_size: " << old_row_size << std::endl;

	// Populate new_offsets based on column-level data.
    for (size_t col_idx = 0; col_idx < columns.size(); ++col_idx) {
	  size_t dynamic_array_size_size = 0; // JTODO
        if (columns[col_idx].get_type() == Data_Type::CHAR) {
            new_columns[col_idx].set_array_size(minimum_column_data_widths[col_idx]);
			// std::cout << "shrink(), col_idx: " << col_idx << ", array_size: " << minimum_column_data_widths[col_idx] << std::endl;
			if (columns[col_idx].get_dynamic_array_flag()) {
			  dynamic_array_size_size = sizeof(uint32_t);
			}
        }
        new_row_size += new_columns[col_idx].data_size();
		new_row_size += dynamic_array_size_size;
		// std::cout << "new_row_size: " << new_row_size << std::endl;
		// std::cout << "pushing back new_offset: " << new_row_size << std::endl;
        new_offsets.push_back(new_row_size);
    } // end loop through columns

    size_t num_rows = data.size() / old_row_size;

    // FIXME: Do this in place.
    std::vector<uint8_t> new_data(num_rows * new_row_size);
    size_t old_row_offset(0), new_row_offset(0);
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
        for (size_t col_idx = 0; col_idx < offsets.size() - 1; ++col_idx) {

		  size_t dynamic_array_size_size = (columns[col_idx].get_dynamic_array_flag() ? sizeof(uint32_t) : 0);

		  // JTODO Don't we need to trim?  How do we know we're copying the relevant part of old_data?
		  // We already trimmed while loading old_data.
            std::copy(data.begin() + old_row_offset + offsets[col_idx],
                      data.begin() + old_row_offset + offsets[col_idx] + dynamic_array_size_size +
                              new_columns[col_idx].data_size(),
                      new_data.begin() + new_row_offset + new_offsets[col_idx]);
        } // end loop through columns
        old_row_offset += old_row_size;
        new_row_offset += new_row_size;
    } // end loop through rows
    using namespace std;
    swap(data, new_data);
    swap(columns, new_columns);
    swap(offsets, new_offsets);
}
