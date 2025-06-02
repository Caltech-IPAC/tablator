#pragma once

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Column.hxx"
#include "Utils/Table_Utils.hxx"  // JTODO

// JTODO: Create struct to hold commonly used (columns, offsets) pair?

namespace tablator {

class Data_Element {
public:
    Data_Element(const std::vector<Column> &columns, const std::vector<size_t> &offsets,
                 const std::vector<uint8_t> &data)
            : columns_(columns), offsets_(offsets), data_(data) {}

    // accessors
    inline const std::vector<Column> &get_columns() const { return columns_; }
    inline std::vector<Column> &get_columns() { return columns_; }

    inline const std::vector<size_t> &get_offsets() const { return offsets_; }
    inline std::vector<size_t> &get_offsets() { return offsets_; }


    inline const std::vector<uint8_t> &get_data() const { return data_; }
    inline std::vector<uint8_t> &get_data() { return data_; }

    inline void set_data(const std::vector<uint8_t> &d) { data_ = d; }
#if 0
  //    inline size_t get_row_size() const { return *offsets_.rbegin(); }
  inline size_t get_row_size() const { return tablator::get_row_size(offsets_); }
    inline size_t get_num_rows() const { return get_data().size() / get_row_size(); }
#endif

#if 1
#if 0
  inline size_t get_num_dynamic_arrays() const { return tablator::get_num_dynamic_arrays(columns_); }
  // JTODO assertion
  inline size_t get_row_size_without_dynamic_array_sizes() const { return tablator::get_row_size_without_dynamic_array_sizes(offsets_, columns_); }
#endif
#else

  inline size_t get_num_dynamic_arrays() const {
	size_t nda=0;
	// JTODO fancy
	for (const auto &column : columns_) {
	  if (column.get_dynamic_array_flag()) {
		++nda;
	  }
	}
	return nda;
  }
  // JTODO assertion
  inline size_t row_size_without_dynamic_array_sizes() const {
	return get_row_size() - get_num_dynamic_arrays() * sizeof(uint32_t);
  }
	#endif
private:
    std::vector<Column> columns_;
    std::vector<size_t> offsets_ = {0};
    std::vector<uint8_t> data_;
};

}  // namespace tablator
