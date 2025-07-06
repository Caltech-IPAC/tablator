#include "../../../Field_Framework.hxx"
#include "../../../Table.hxx"

#include "../../read_ipac_table.hxx"

// As of 28Jun25, all columns are created with dynamic_array_flag value <false>.
// JTODO rename function and ipac_columns parameter.
namespace tablator {

Field_Framework Table::create_types_from_ipac_headers(
        const std::array<std::vector<std::string>, 4> &ipac_columns,
        const std::vector<size_t> &ipac_column_widths) {
    std::vector<Column> orig_columns;
    orig_columns.emplace_back(ipac_columns.at(COL_NAME_IDX).at(0), Data_Type::UINT8_LE,
                              ipac_column_widths.at(0),
                              Field_Properties(null_bitfield_flags_description));

    // ipac_columns, like orig_columns, includes a null column.
    size_t num_ipac_columns = ipac_columns[0].size();
    for (size_t col_idx = 1; col_idx < num_ipac_columns; ++col_idx) {
        append_ipac_data_member(orig_columns, ipac_columns.at(COL_NAME_IDX).at(col_idx),
                                ipac_columns.at(COL_TYPE_IDX).at(col_idx),
                                ipac_column_widths.at(col_idx));
    }
    Field_Framework field_framework(orig_columns, true /* got_null_bitfields_column */);
    auto &columns = field_framework.get_columns();
    for (size_t col_idx = 1; col_idx < num_ipac_columns; ++col_idx) {
        Field_Properties field_props({});
        if (!ipac_columns[COL_UNIT_IDX].at(col_idx).empty()) {
            field_props.add_attribute("unit",
                                      boost::algorithm::trim_copy(
                                              ipac_columns[COL_UNIT_IDX].at(col_idx)));
        }
        if (!ipac_columns[COL_NULL_IDX].at(col_idx).empty()) {
            field_props.get_values().null =
                    boost::algorithm::trim_copy(ipac_columns[COL_NULL_IDX].at(col_idx));
        }
        columns.at(col_idx).set_field_properties(field_props);
    }
    return field_framework;
}

}  // namespace tablator
