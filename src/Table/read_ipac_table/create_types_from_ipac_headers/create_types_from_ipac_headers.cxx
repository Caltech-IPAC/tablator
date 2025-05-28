#include "../../../Table.hxx"

#include "../../read_ipac_table.hxx"

void tablator::Table::create_types_from_ipac_headers(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        const std::array<std::vector<std::string>, 4> &ipac_columns,
        const std::vector<size_t> &ipac_column_widths) {
    tablator::append_column(columns, offsets, ipac_columns.at(COL_NAME_IDX).at(0),
                            Data_Type::UINT8_LE, ipac_column_widths.at(0),
                            Field_Properties(null_bitfield_flags_description));

    size_t num_columns = ipac_columns[0].size();
    for (size_t col_idx = 1; col_idx < num_columns; ++col_idx) {
        append_ipac_data_member(columns, offsets,
                                ipac_columns.at(COL_NAME_IDX).at(col_idx),
                                ipac_columns.at(COL_TYPE_IDX).at(col_idx),
                                ipac_column_widths.at(col_idx));
    }

    for (size_t col_idx = 1; col_idx < num_columns; ++col_idx) {
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
}
