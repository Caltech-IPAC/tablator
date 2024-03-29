#include "../Ascii_Writer.hxx"
#include "../Table.hxx"


std::string tablator::Table::extract_value_as_string(
        const std::string &col_name, size_t row_id,
        const Command_Line_Options &options) const {
    size_t col_id = column_index(col_name);  // throws if col_name is invalid
    return extract_value_as_string(col_id, row_id, options);
}


std::string tablator::Table::extract_value_as_string(
        size_t col_id, size_t row_id, const Command_Line_Options &options) const {
    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    const auto &data = get_data();
    if ((col_id == 0) || (col_id >= columns.size())) {
        throw std::runtime_error("Invalid column index: " + std::to_string(col_id));
    }
    if (row_id >= num_rows()) {
        throw std::runtime_error("Invalid row index: " + std::to_string(row_id));
    }

    size_t curr_row_offset = row_id * row_size();
    auto &column = columns[col_id];
    if (is_null(curr_row_offset, col_id)) {
        // JTODO Or leave blank?  Or use values from get_null() (if they aren't already
        // there)?
        auto &null_value = column.get_field_properties().get_values().null;
        return (null_value.empty() ? tablator::Table::DEFAULT_NULL_VALUE : null_value);
    }
    std::stringstream ss;

    // JTODO Write UINT8_LE values the way Ipac_Table_Writer does?
    tablator::Ascii_Writer::write_type_as_ascii(
            ss, column.get_type(), column.get_array_size(),
            data.data() + curr_row_offset + offsets[col_id],
            tablator::Ascii_Writer::DEFAULT_SEPARATOR, options);
    return ss.str();
}

//==================================================================

std::vector<std::string> tablator::Table::extract_column_values_as_strings(
        const std::string &col_name, const Command_Line_Options &options) const {
    size_t col_id = column_index(col_name);  // throws if col_name is invalid

    std::vector<std::string> col_vals;

    for (size_t curr_row_id = 0; curr_row_id < num_rows(); ++curr_row_id) {
        col_vals.emplace_back(extract_value_as_string(col_id, curr_row_id, options));
    }
    return col_vals;
}
