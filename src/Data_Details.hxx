#pragma once

#include <set>

#include "Common.hxx"
#include "Row.hxx"

namespace tablator {

class Field_Framework;

class Data_Details {
public:
    Data_Details(const size_t &num_dynamic_columns, const size_t &row_size, const size_t &num_rows = 0)
            : num_dynamic_columns_(num_dynamic_columns), row_size_(row_size) {
	  //	  init(num_dynamic_columns, row_size, num_rows);
	  reserve_rows(num_rows);
         // std::cout << "DD(), get_num_dynamic_columns(): " << get_num_dynamic_columns() << ", num_rows: " << num_rows << std::endl;
    }

    Data_Details(const Field_Framework &field_framework, const size_t &num_rows = 0);

    void append_row(Row &row, const Field_Framework &field_framework);

    void append_row(Row &row);

    // This function is called only by Table::append_rows(), which
    // checks that the Field_Frameworks of the two tables are
    // compatible.
    void append_rows(const Data_Details &other);


    void add_cntr_column(const Field_Framework &dest_ff, const Field_Framework &src_ff,
                         const Data_Details &src_dd);

    void combine_data_details(const Field_Framework &dest_ff,
                              const Field_Framework &src1_ff,
                              const Field_Framework &src2_ff,
                              const Data_Details &src1_dd, const Data_Details &src2_dd);


  void winnow_rows(const std::set<size_t> &selected_row_idx_list);

    inline void adjust_num_rows(size_t new_num_rows) {
	  data_.resize(new_num_rows);
	  dynamic_array_sizes_by_row_.resize(new_num_rows);
    }

#if 0
    // deprecated
    void resize_data(size_t new_num_rows) { adjust_num_rows(new_num_rows); }
#endif

  // JTODO reserve
    inline void reserve_rows(const size_t &new_num_rows) {
	  data_.reserve(/* get_row_size() * */ new_num_rows);
	  dynamic_array_sizes_by_row_.reserve(new_num_rows);
    }
    // accessors

  inline const std::vector<std::vector<char>> &get_data() const { return data_; }

    // Non-const to support append_rows().
  inline std::vector<std::vector<char>> &get_data() { return data_; }

  inline void set_data(const std::vector<std::vector<char>> &d) { data_ = d; }

    inline size_t get_num_dynamic_columns() const { return num_dynamic_columns_; }
  inline bool got_dynamic_columns() const { return num_dynamic_columns_ > 0; }

#if 0
    inline size_t get_data_size() const { return data_.size(); }
#endif

  inline size_t get_num_rows() const { return data_.size(); }

    inline size_t get_row_size() const { return row_size_; }


    inline const std::vector<std::vector<uint32_t>> &get_dynamic_array_sizes_by_row()
            const {
        return dynamic_array_sizes_by_row_;
    }
    inline std::vector<std::vector<uint32_t>> &get_dynamic_array_sizes_by_row() {
        return dynamic_array_sizes_by_row_;
    }

    inline const std::vector<uint32_t> &get_dynamic_array_sizes(size_t row_idx) const {
        return get_dynamic_array_sizes_by_row().at(row_idx);
    }


private:
#if 0
  // JTODO add this fix to first commit
  void init(const size_t &num_dynamic_columns, const size_t &row_size, const size_t &num_rows = 0) {
        reserve_rows(num_rows);
		for (uint i = 0; i < num_rows; ++i) {
		  data_.emplace_back();
		  data_.back().reserve(row_size);
		}


        if (num_dynamic_columns > 0) {
            dynamic_array_sizes_by_row_.reserve(num_rows);

            for (uint i = 0; i < num_rows; ++i) {
                dynamic_array_sizes_by_row_.emplace_back();
                dynamic_array_sizes_by_row_.back().reserve(num_dynamic_columns);
            }
        }
    }
#endif
    // Can't be const because of append_rows().
    // JTODO data_ could also be made 2-dim'l.

  // JTODO could simply have vector of Row.

  std::vector<std::vector<char>> data_;
    std::vector<std::vector<uint32_t>> dynamic_array_sizes_by_row_;
    size_t num_dynamic_columns_;
    size_t row_size_;
};

}  // namespace tablator
