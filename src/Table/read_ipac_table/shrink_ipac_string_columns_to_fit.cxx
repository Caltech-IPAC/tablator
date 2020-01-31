#include "../../Table.hxx"

#include <stdexcept>
#include <utility>

void tablator::Table::shrink_ipac_string_columns_to_fit(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        std::vector<uint8_t> &data, const std::vector<size_t> &column_widths) {
    std::vector<size_t> new_offsets = {0};
    std::vector<Column> new_columns(columns);

    size_t old_row_size(tablator::row_size(offsets));
    size_t new_row_size(0);

    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].get_type() == Data_Type::CHAR) {
            new_columns[i].set_array_size(column_widths[i]);
        }
        new_row_size += new_columns[i].data_size();
        new_offsets.push_back(new_row_size);
    }

    size_t num_rows = data.size() / old_row_size;

    // FIXME: Do this in place.
    std::vector<uint8_t> new_data(num_rows * new_row_size);
    size_t old_row_offset(0), new_row_offset(0);
    for (size_t row_idx = 0; row_idx < num_rows; ++row_idx) {
        for (size_t col_idx = 0; col_idx < offsets.size() - 1; ++col_idx) {
            std::copy(data.begin() + old_row_offset + offsets[col_idx],
                      data.begin() + old_row_offset + offsets[col_idx] +
                              new_columns[col_idx].data_size(),
                      new_data.begin() + new_row_offset + new_offsets[col_idx]);
        }
        old_row_offset += old_row_size;
        new_row_offset += new_row_size;
    }
    using namespace std;
    swap(data, new_data);
    swap(columns, new_columns);
    swap(offsets, new_offsets);
}
