#include "../Row.hxx"

#include "../Field_Framework.hxx"
#include "../Table.hxx"

namespace tablator {

Row::Row(const Table &table) : Row(table.get_field_framework()) {}

Row::Row(const Field_Framework &field_framework)
        : data_(field_framework.get_row_size()),
          dynamic_col_idx_lookup_(field_framework.get_dynamic_col_idx_lookup()),
          dynamic_array_sizes_(field_framework.get_num_dynamic_columns()) {}

}  // namespace tablator
