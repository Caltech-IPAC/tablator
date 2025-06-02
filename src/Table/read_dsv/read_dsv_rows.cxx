#include "../../Table.hxx"

std::vector<uint8_t> tablator::Table::read_dsv_rows(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        const std::list<std::vector<std::string> > &dsv) {
    bool skipped(false);

    Row row_string(tablator::get_row_size(offsets));
    std::vector<uint8_t> data;
    for (auto &dsv_row : dsv) {
        if (!skipped) {
            skipped = true;
            continue;
        }
        row_string.fill_with_zeros();
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
			const auto &column = columns[col_idx];
            const std::string &element(dsv_row[col_idx - 1]);
            if (element.empty() || element == "null") {
                row_string.set_null(column.get_type(),
                                    column.get_array_size(), col_idx,
                                    offsets[col_idx], offsets[col_idx + 1]);
            } else {
			  // JTODO
			  insert_ascii_in_row(row_string, column.get_type(),
                                    column.get_array_size(), col_idx, element,
								  offsets[col_idx], offsets[col_idx + 1], /* false */ column.get_dynamic_array_flag());
            }
        }
        tablator::append_row(data, row_string);
    }
    return data;
}
