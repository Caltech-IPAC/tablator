#pragma once

#include "../Column.hxx"
#include "../Row.hxx"

namespace tablator {

static const std::string null_bitfield_flags_name("null_bitfield_flags");
static const std::string null_bitfield_flags_description(
        "Packed bit array indicating whether an entry is null");

// Computes the minimum number of bytes which collectively contain
// n distinct bits.
inline size_t bits_to_bytes(size_t n) { return (n + 7) / 8; }

inline size_t get_row_size(const std::vector<size_t> &offsets) {
    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
    return offsets.back();
}

  inline size_t get_num_rows(const std::vector<size_t> &offsets, const std::vector<uint8_t> &data) {
    if (offsets.empty()) {
        throw std::runtime_error("<offsets> is empty");
    }
    return data.size()/get_row_size(offsets);
}

  inline size_t get_num_dynamic_arrays(const std::vector<Column> &columns) {
	size_t nda=0;
	// JTODO fancy
	for (const auto &column : columns) {
	  if (column.get_dynamic_array_flag()) {
		++nda;
	  }
	}
	return nda;
  }
  // JTODO assertion
  inline size_t get_row_size_without_dynamic_array_sizes(const std::vector<size_t> &offsets, const std::vector<Column> &columns) {
	// std::cout << "get_row_size_without(), enter" << std::endl;
	// std::cout << "  row_size(): " << get_row_size(offsets) << std::endl;
	// std::cout << "  num_dynamic_arrays(): " << get_num_dynamic_arrays(columns) << std::endl;
	// std::cout << "  return row_size_wo: " << get_row_size(offsets) - get_num_dynamic_arrays(columns) * sizeof(uint32_t) << std::endl;
	return get_row_size(offsets) - get_num_dynamic_arrays(columns) * sizeof(uint32_t);
  }



void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                   const Column &column);

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type,
                          const size_t &size, const Field_Properties &field_properties,
                          bool dynamic_array_flag) {
    append_column(columns, offsets,
                  Column(name, type, size, field_properties, dynamic_array_flag));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type,
                          const size_t &size,
                          const Field_Properties &field_properties) {
  // std::cout << "append_column(), setting flag to " << (type == Data_Type::CHAR) << std::endl;
    append_column(columns, offsets,
                  Column(name, type, size, field_properties,
                         type == Data_Type::CHAR /* dynamic_array_flag */));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type,
                          const size_t &size) {
    append_column(columns, offsets, Column(name, type, size, Field_Properties()));
}

inline void append_column(std::vector<Column> &columns, std::vector<size_t> &offsets,
                          const std::string &name, const Data_Type &type) {
    append_column(columns, offsets, name, type, 1, Field_Properties());
}

inline void append_row(std::vector<uint8_t> &data, const Row &row) {
    data.insert(data.end(), row.data.begin(), row.data.end());
}

inline void unsafe_append_row(std::vector<uint8_t> &data, const char *row,
                              uint row_size) {
    data.insert(data.end(), row, row + row_size);
}

inline void pop_row(std::vector<uint8_t> &data, uint row_size) {
    data.resize(data.size() - row_size);
}

inline void resize_rows(std::vector<uint8_t> &data, const size_t &new_num_rows,
                        uint row_size) {
    data.resize(new_num_rows * row_size);
}

  void insert_ascii_in_row(Row &row, const Data_Type &data_type, const size_t &array_size,
                         const size_t &column, const std::string &element,
                         const size_t &offset, const size_t &offset_end, bool dynamic_array_flag);

}  // namespace tablator
