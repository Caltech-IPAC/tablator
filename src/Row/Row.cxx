#include "../Row.hxx"

#include "../Field_Framework.hxx"
#include "../Table.hxx"

namespace tablator {

Row::Row(const Table &table)
        : Row(table.get_row_size(), table.get_num_dynamic_columns()) {}

Row::Row(const Field_Framework &field_framework)
        : Row(field_framework.get_row_size(),
              field_framework.get_num_dynamic_columns()) {}
}  // namespace tablator
