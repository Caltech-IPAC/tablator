#include "../Table_Utils.hxx"

void tablator::append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                             const Column &column) {
    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
	// std::cout << "TU::append_column(), get_dynamic_array_flag(): " << column.get_dynamic_array_flag() << std::endl;
    auto new_columns(columns);
    new_columns.push_back(column);
    size_t old_row_size = *offsets.rbegin();
    size_t new_row_size = old_row_size + new_columns.rbegin()->get_data_size();
	// std::cout << "new_row_size, orig: " << new_row_size << std::endl;
	if (column.get_dynamic_array_flag()) {
	  new_row_size += sizeof(uint32_t);
	}
	// std::cout << "new_row_size, final: " << new_row_size << std::endl;
    auto new_offsets(offsets);
    new_offsets.push_back(new_row_size);

    /// Copy and swap for exception safety.
    using namespace std;
    swap(columns, new_columns);
    swap(offsets, new_offsets);
}
