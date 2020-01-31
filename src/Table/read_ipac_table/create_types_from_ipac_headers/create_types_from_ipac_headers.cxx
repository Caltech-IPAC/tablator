#include "../../../Table.hxx"


void tablator::Table::create_types_from_ipac_headers(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        const std::array<std::vector<std::string>, 4> &ipac_columns,
        const std::vector<size_t> &ipac_column_widths) {
    tablator::append_column(columns, offsets, ipac_columns.at(0).at(0),
                            Data_Type::UINT8_LE, ipac_column_widths.at(0),
                            Field_Properties(null_bitfield_flags_description));

    size_t num_columns = ipac_columns[0].size();
    for (size_t i = 1; i < num_columns; ++i) {
        // JTODO magic numbers
        append_ipac_data_member(columns, offsets, ipac_columns.at(0).at(i),
                                ipac_columns.at(1).at(i), ipac_column_widths.at(i));
    }

    for (size_t column = 1; column < num_columns; ++column) {
        Field_Properties field_props({});
        if (!ipac_columns[2].at(column).empty()) {
            field_props.add_attribute(
                    "unit", boost::algorithm::trim_copy(ipac_columns[2].at(column)));
        }
        if (!ipac_columns[3].at(column).empty()) {
            field_props.get_values().null =
                    boost::algorithm::trim_copy(ipac_columns[3].at(column));
        }
        columns.at(column).set_field_properties(field_props);
    }
}
