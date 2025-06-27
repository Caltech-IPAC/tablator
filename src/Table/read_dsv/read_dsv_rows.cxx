#include "../../Table.hxx"

std::vector<uint8_t> tablator::Table::read_dsv_rows(Field_Framework &field_framework,
													const std::list<std::vector<std::string> > &dsv) {
    bool skipped(false);

    std::vector<uint8_t> data;

    auto &columns = field_framework.get_columns();
    auto &offsets = field_framework.get_offsets();

    Row row_string(tablator::get_row_size(offsets));
    for (auto &dsv_row : dsv) {
        if (!skipped) {
            skipped = true;
            continue;
        }
        row_string.fill_with_zeros();
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            const std::string &element(dsv_row[col_idx - 1]);
            if (element.empty() || element == "null") {
                row_string.insert_null(columns[col_idx].get_type(),
                                       columns[col_idx].get_array_size(), col_idx,
                                       offsets[col_idx], offsets[col_idx + 1]);
            } else {
                insert_ascii_in_row(row_string, columns[col_idx].get_type(),
                                    columns[col_idx].get_array_size(), col_idx, element,
                                    offsets[col_idx], offsets[col_idx + 1]);
            }
        }
        tablator::append_row(data, row_string);
    }
    return data;
}
