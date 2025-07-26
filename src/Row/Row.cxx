#include "../Row.hxx"

#include "../Field_Framework.hxx"
#include "../Table.hxx"

namespace tablator {

Row::Row(const Field_Framework &field_framework)
        : data_(field_framework.get_row_size()) {}

Row::Row(const Table &table) : Row(table.get_field_framework()) {}

}  // namespace tablator
