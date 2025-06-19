#include "../Table_Utils.hxx"

void tablator::append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                             const Column &column) {
    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
    auto new_columns(columns);
    new_columns.push_back(column);
    size_t old_row_size = *offsets.rbegin();
    size_t new_row_size = old_row_size + new_columns.rbegin()->get_data_size();

    if (column.get_dynamic_array_flag()) {
        new_row_size += tablator::DYNAMIC_ARRAY_OFFSET;
    }
    auto new_offsets(offsets);
    new_offsets.push_back(new_row_size);

    /// Copy and swap for exception safety.
    using namespace std;
    swap(columns, new_columns);
    swap(offsets, new_offsets);
}
