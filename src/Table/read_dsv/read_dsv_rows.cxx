#include "../../Table.hxx"

std::vector<uint8_t> tablator::Table::read_dsv_rows(
        Field_Framework &field_framework,
        const std::list<std::vector<std::string> > &dsv) {
    bool skipped(false);

    std::vector<uint8_t> data;

    auto &columns = field_framework.get_columns();
    auto &offsets = field_framework.get_offsets();

    Row single_row(tablator::get_row_size(offsets));
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
