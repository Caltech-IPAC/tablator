#include "../../Table.hxx"

#include <stdexcept>
#include <utility>

void tablator::Table::shrink_ipac_string_columns_to_fit(
        Field_Framework &old_field_framework, Data_Details &old_data_details,
        const std::vector<size_t> &minimum_column_data_widths) {
    auto &old_columns = old_field_framework.get_columns();
    auto &old_offsets = old_field_framework.get_offsets();
    size_t num_columns = old_columns.size();
    size_t old_row_size(old_field_framework.get_row_size());

    std::vector<uint8_t> &old_data = old_data_details.get_data();
    size_t num_rows = old_data_details.get_num_rows();

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

    if (old_row_size == new_row_size) {
        // Nothing can be shrunk.
        return;
    }

    // FIXME: Do this in place.
    Data_Details new_data_details(new_row_size, num_rows);
    std::vector<uint8_t> &new_data = new_data_details.get_data();

    size_t old_row_offset(0), new_row_offset(0);
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
        for (size_t col_idx = 0; col_idx < num_columns; ++col_idx) {
            std::copy(old_data.begin() + old_row_offset + old_offsets[col_idx],
                      old_data.begin() + old_row_offset + old_offsets[col_idx] +
                              new_columns[col_idx].get_data_size(),
                      std::back_inserter(new_data));
        }
        old_row_offset += old_row_size;
        new_row_offset += new_row_size;
    }

    std::swap(old_field_framework, new_field_framework);
    std::swap(old_data_details, new_data_details);
}
