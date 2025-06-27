#include "../Table_Utils.hxx"
#if 0
void tablator::append_column(Field_Framework &field_framework, const Column &column) {
    auto &offsets = field_framework.get_offsets();

    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
    auto &columns = field_framework.get_columns();

    auto new_columns(columns);
    new_columns.push_back(column);
    size_t old_row_size = *offsets.rbegin();
    size_t new_row_size = old_row_size + new_columns.rbegin()->get_data_size();
    auto new_offsets(offsets);
    new_offsets.push_back(new_row_size);

    /// Copy and swap for exception safety.
    using namespace std;
    swap(columns, new_columns);
    swap(offsets, new_offsets);
}
#endif
