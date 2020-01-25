#include "../Table.hxx"

void tablator::Table::append_column(std::vector<Column> &columns,
                                    std::vector<size_t> &offsets,
                                    const Column &column) {
    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
    auto new_columns(columns);
    new_columns.push_back(column);
    size_t old_row_size = *offsets.rbegin();
    size_t new_row_size = old_row_size + new_columns.rbegin()->data_size();
    auto new_offsets(offsets);
    new_offsets.push_back(new_row_size);

    /// Copy and swap for exception safety.
    using namespace std;
    swap(columns, new_columns);
    swap(offsets, new_offsets);
}
