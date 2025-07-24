#include "../../Table.hxx"

#include <stdexcept>
#include <utility>

// No need to adjust dynamic_array_sizes for now, since we don't
// support dynamic array columns in IPAC tables.  13Jul25

void tablator::Table::shrink_ipac_string_columns_to_fit(
        Field_Framework &old_field_framework, Data_Details &old_data_details,
        const std::vector<size_t> &minimum_column_data_widths) {
	  // std::cout << "shrink(), enter" << std::endl;
    auto &old_columns = old_field_framework.get_columns();
    auto &old_offsets = old_field_framework.get_offsets();
    size_t num_columns = old_columns.size();
    size_t old_row_size(old_field_framework.get_row_size());

    for (size_t col_idx = 0; col_idx < num_columns; ++col_idx) {
        // Populate new_offsets based on column-level data.
        if (old_columns[col_idx].get_type() == Data_Type::CHAR) {
            old_columns[col_idx].set_array_size(minimum_column_data_widths[col_idx]);
        }
    }

    Field_Framework new_field_framework(old_columns,
                                        true /* got_null_bitfields_column */);
    std::vector<Column> &new_columns = new_field_framework.get_columns();
    std::vector<size_t> &new_offsets = new_field_framework.get_offsets();
    size_t new_row_size = new_field_framework.get_row_size();
	// std::cout << "shrink(), old_row_size: " << old_row_size << ", new_row_size: " << new_row_size << std::endl;

    if (old_row_size == new_row_size) {
        // Nothing can be shrunk.
	  // std::cout << "shrink(), early exit" << std::endl;
        return;
    }

    size_t num_rows = old_data_details.get_num_rows();
#if 0
    std::vector<uint8_t> &old_data = old_data_details.get_data();
#else
	std::vector<std::vector<char>> &old_data = old_data_details.get_data();
#endif
    // FIXME: Do this in place.
    Data_Details new_data_details(old_data_details.get_num_dynamic_columns(),
                                  new_row_size, num_rows);
	std::vector<std::vector<char>> &new_data = new_data_details.get_data();
	// std::cout << "shrink(), after creating new_dd, new_data.size(): " << new_data.size() << std::endl;
#if 0
    size_t old_row_offset(0), new_row_offset(0);
#endif
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {

			new_data.emplace_back();
			new_data.back().reserve(new_row_size);

        for (size_t col_idx = 0; col_idx < num_columns; ++col_idx) {
#if 0
            std::copy(old_data.begin() + old_row_offset + old_offsets[col_idx],
                      old_data.begin() + old_row_offset + old_offsets[col_idx] +
                              new_columns[col_idx].get_data_size(),
                      std::back_inserter(new_data));
#else
			//			std::cout << "shrink(), before copy" << std::endl;

            std::copy(old_data.at(row_idx).begin() + old_offsets[col_idx],
                      old_data.at(row_idx).begin() + old_offsets[col_idx] +
                              new_columns[col_idx].get_data_size(),
                      std::back_inserter(new_data.at(row_idx)));
			//			std::cout << "shrink(), after copy" << std::endl;
#endif

        }
#if 0
        old_row_offset += old_row_size;
        new_row_offset += new_row_size;
#endif
    }
	// std::cout << "shrink(), before swap, old_dd.size(): " << old_data_details.get_data().size() << ", new_data.size(): " << new_data.size() << std::endl;
	// std::cout << "shrink(), before swap, old_dd row_size(): " << old_data_details.get_row_size() << ", new_dd row_size(): " << new_data_details.get_row_size() << std::endl;
    std::swap(old_field_framework, new_field_framework);
    std::swap(old_data_details, new_data_details);

	// swap back dynamic col info
	std::swap(old_data_details.get_dynamic_array_sizes_by_row(), new_data_details.get_dynamic_array_sizes_by_row());

	// std::cout << "shrink(), after swap, old_dd row_size(): " << old_data_details.get_row_size() << ", new_dd row_size(): " << new_data_details.get_row_size() << std::endl;
}
