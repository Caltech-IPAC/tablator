#include "../../Table.hxx"

std::vector<uint8_t> tablator::Table::read_dsv_rows(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        const std::list<std::vector<std::string> > &dsv) {
    bool skipped(false);

	size_t row_size = tablator::get_row_size(offsets);
    Row single_row(row_size);
    std::vector<uint8_t> data;
	data.reserve(row_size * dsv.size());

    for (auto &dsv_row : dsv) {
        if (!skipped) {
            skipped = true;
            continue;
        }
        single_row.fill_with_zeros();
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            const std::string &element(dsv_row[col_idx - 1]);
            if (element.empty() || element == "null") {
                single_row.insert_null(columns[col_idx].get_type(),
                                       columns[col_idx].get_array_size(), col_idx,
                                       offsets[col_idx], offsets[col_idx + 1]);
            } else {
                single_row.insert_from_ascii(element, columns[col_idx].get_type(),
                                             columns[col_idx].get_array_size(), col_idx,
                                             offsets[col_idx], offsets[col_idx + 1]);
            }
        }
        tablator::append_row(data, single_row);
    }
    return data;
}
