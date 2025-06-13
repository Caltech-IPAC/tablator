#include "../../Table.hxx"

#include <stdexcept>
#include <utility>

void tablator::Table::shrink_ipac_string_columns_to_fit(
        std::vector<Column> &old_columns, std::vector<size_t> &old_offsets,
        const std::vector<size_t> &optimal_column_data_widths) {
													
    std::vector<size_t> new_offsets = {0};
    std::vector<Column> new_columns(old_columns);
    size_t new_row_size(0);

	//============================================
	//  Update offsets
    //============================================

	// Populate new_offsets based on what we learned about CHAR
	// columns by iterating through the data: namely,
	// dynamic_array_flag and minimum_column_data_width.
    for (size_t col_idx = 0; col_idx < old_columns.size(); ++col_idx) {
	  size_t extra_bytes_for_dynamic_array = 0; // JTODO
        if (old_columns[col_idx].get_type() == Data_Type::CHAR) {
            new_columns[col_idx].set_array_size(optimal_column_data_widths[col_idx]);
			if (old_columns[col_idx].get_dynamic_array_flag()) {
			  extra_bytes_for_dynamic_array = sizeof(uint32_t);
			}
        }
        new_row_size += new_columns[col_idx].get_data_size();
		new_row_size += extra_bytes_for_dynamic_array;
        new_offsets.push_back(new_row_size);
    } // end loop through columns

    using namespace std;
	swap(old_columns, new_columns);
    swap(old_offsets, new_offsets);
}
