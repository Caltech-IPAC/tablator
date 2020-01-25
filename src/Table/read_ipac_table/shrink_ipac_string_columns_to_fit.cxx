#include "../../Table.hxx"

#include <stdexcept>
#include <utility>

void tablator::Table::shrink_ipac_string_columns_to_fit(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        std::vector<uint8_t> &data,

        const std::vector<size_t> &column_widths) {
    std::vector<size_t> new_offsets = {0};
    std::vector<Column> new_columns(columns);


    size_t old_row_size(row_size(offsets));
    size_t num_rows = data.size() / old_row_size;

    size_t new_row_size(0);
    for (size_t i = 0; i < columns.size(); ++i) {
        if (columns[i].get_type() == Data_Type::CHAR) {
            new_columns[i].set_array_size(column_widths[i]);
        }
        new_row_size += new_columns[i].data_size();
        new_offsets.push_back(new_row_size);
    }

    // FIXME: Do this in place.
    std::vector<uint8_t> new_data(num_rows * new_row_size);
    size_t old_row_offset(0), new_row_offset(0);
    for (size_t idx = 0; idx < num_rows; ++idx) {
        for (size_t column = 0; column < offsets.size() - 1; ++column) {
            std::copy(data.begin() + old_row_offset + offsets[column],
                      data.begin() + old_row_offset + offsets[column] +
                              new_columns[column].data_size(),
                      new_data.begin() + new_row_offset + new_offsets[column]);
        }
        old_row_offset += old_row_size;
        new_row_offset += new_row_size;
    }
    using namespace std;
    swap(data, new_data);
    swap(columns, new_columns);
    swap(offsets, new_offsets);
}
