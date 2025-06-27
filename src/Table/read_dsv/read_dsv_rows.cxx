#include "../../Table.hxx"

std::vector<uint8_t> tablator::Table::read_dsv_rows(
        Field_Framework &field_framework,
        const std::list<std::vector<std::string> > &dsv) {
    // JTODO this function should not be a Table class member.
    auto &columns = field_framework.get_columns();
    auto &offsets = field_framework.get_offsets();
    size_t row_size = field_framework.get_row_size();

    std::vector<uint8_t> data;
    data.reserve(row_size * dsv.size());


    Row single_row(row_size);
    bool skipped(false);
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
