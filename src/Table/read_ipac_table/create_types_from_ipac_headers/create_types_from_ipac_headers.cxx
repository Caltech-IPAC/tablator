#include "../../../Table.hxx"

#include "../../read_ipac_table.hxx"

// JTODO rename file
void tablator::Table::create_types_from_ipac_headers(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        const std::array<std::vector<std::string>, 4> &incoming_columns,
        const std::vector<size_t> &incoming_column_widths) {
  // Create initial null_bifield_flags column.
    tablator::append_column(columns, offsets, incoming_columns.at(COL_NAME_IDX).at(0),
                            Data_Type::UINT8_LE, incoming_column_widths.at(0),
                            Field_Properties(null_bitfield_flags_description));

    size_t num_columns = incoming_columns[COL_NAME_IDX].size();
    for (size_t col_idx = 1; col_idx < num_columns; ++col_idx) {
	  auto col_type = incoming_columns[COL_TYPE_IDX].at(col_idx);
	  append_ipac_data_member(columns, offsets,
							  incoming_columns.at(COL_NAME_IDX).at(col_idx),
							  incoming_columns.at(COL_TYPE_IDX).at(col_idx),
							  incoming_column_widths.at(col_idx));
    }

    for (size_t col_idx = 1; col_idx < num_columns; ++col_idx) {
        Field_Properties field_props({});
        if (!incoming_columns[COL_UNIT_IDX].at(col_idx).empty()) {
            field_props.add_attribute("unit",
                                      boost::algorithm::trim_copy(
                                              incoming_columns[COL_UNIT_IDX].at(col_idx)));
        }
        if (!incoming_columns[COL_NULL_IDX].at(col_idx).empty()) {
            field_props.get_values().null =
                    boost::algorithm::trim_copy(incoming_columns[COL_NULL_IDX].at(col_idx));
        }
        columns.at(col_idx).set_field_properties(field_props);
    }
}
