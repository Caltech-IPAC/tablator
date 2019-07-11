#include "../Table.hxx"

// The IPAC table format does not support array-valued non-char columns.
// A array column of fixed size n named "col" of non-char type is formatted
// as single-valued columns named "col_0", "col_1", ... "col_{n-1}".

// This function does the right thing for columns of type UINT64_LE even if
// their active_datatype is CHAR.

std::vector<size_t> tablator::Ipac_Table_Writer::get_column_widths(
        const tablator::Table &table) {
    std::vector<size_t> widths;
    auto columns = table.columns;
    auto column_iter(std::next(columns.begin()));
    // First column is the null bitfield flags, which are not written
    // out in ipac_tables.
    widths.push_back(0);
    for (; column_iter != columns.end(); ++column_iter) {
        size_t header_size(
                column_iter->name.size() +
                (column_iter->array_size == 1
                         ? 0
                         : 1 + std::to_string(column_iter->array_size - 1).size()));
        auto unit = column_iter->field_properties.attributes.find("unit");
        if (unit != column_iter->field_properties.attributes.end()) {
            header_size = std::max(header_size, unit->second.size());
        }
        if (column_iter->type == Data_Type::CHAR) {
            // The minimum of 4 is to accomodate the length of the
            // literals 'char' and 'null'.
            widths.push_back(std::max((size_t)4,
                                      std::max(header_size, column_iter->array_size)));
        } else {
            // buffer_size = 1 (sign) + 1 (leading digit) + 1
            // (decimal) + 1 (exponent sign) + 3 (exponent) (value
            // could be e.g. uint64 as well as double, but double's
            // buffer_size is generous enough).
            const size_t buffer_size(7);
            widths.push_back(
                    std::max(header_size,
                             std::numeric_limits<double>::max_digits10 + buffer_size));
        }
    }
    return widths;
}
