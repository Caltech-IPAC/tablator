#pragma once

#include "Column.hxx"

// JTODO: Create struct to hold commonly used (columns, offsets) pair?

namespace tablator {

class Field_Framework {
public:
    Field_Framework(std::vector<Column> &columns, std::vector<size_t> &offsets)
            : columns_(columns), offsets_(offsets) {}

  Field_Framework() : offsets_({0}) {}

    // accessors
    inline const std::vector<Column> &get_columns() const { return columns_; }
    inline std::vector<Column> &get_columns() { return columns_; }

    inline const std::vector<size_t> &get_offsets() const { return offsets_; }
    inline std::vector<size_t> &get_offsets() { return offsets_; }



private:
    std::vector<Column> columns_;
  std::vector<size_t> offsets_;
};

}  // namespace tablator
