#include "../Table_Utils.hxx"

#include <stdexcept>
#include <string>

void tablator::winnow_rows(std::vector<uint8_t> &data,
                           const std::set<size_t> &selected_row_idx_list,
                           size_t num_rows, size_t row_size) {
    if (data.size() != num_rows * row_size) {
        throw std::runtime_error(
                "Mismatch between data.size(), num_rows, and row_size.");
    }
    size_t num_selected_rows = selected_row_idx_list.size();
    if (num_selected_rows > num_rows) {
        throw std::runtime_error("Number of selected rows must not exceed " +
                                 std::to_string(num_rows));
    }
    if (*selected_row_idx_list.rbegin() >= num_rows) {
        throw std::runtime_error("invalid row index: " +
                                 std::to_string(*selected_row_idx_list.rbegin()));
    }
    if (num_selected_rows == num_rows) {
        return;
    }

    const auto data_start_ptr = data.data();
    auto read_ptr = data_start_ptr;
    auto write_ptr = data_start_ptr;
    size_t write_idx = 0;
    size_t prev_row_idx = 0;

    for (auto row_idx : selected_row_idx_list) {
        if (row_idx >= write_idx) {
            read_ptr += ((row_idx - prev_row_idx) * row_size);
            if (row_idx > write_idx) {
                // Copy row_idx-th row to begin immediately after the end of the
                // previous copied row.
                std::copy(read_ptr, read_ptr + row_size, write_ptr);
            }
            ++write_idx;
            write_ptr += row_size;
            prev_row_idx = row_idx;
        }
    }

    // Delete all data past the last row copied.
    data.resize(std::distance(data_start_ptr, write_ptr));
}
