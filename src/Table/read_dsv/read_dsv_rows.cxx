#include "../../Table.hxx"

std::vector<uint8_t> tablator::Table::read_dsv_rows(
        std::vector<Column> &columns, std::vector<size_t> &offsets,
        const std::list<std::vector<std::string> > &dsv) {
    bool skipped(false);

    Row row_string(tablator::row_size(offsets));
    std::vector<uint8_t> data;
    for (auto &dsv_row : dsv) {
        if (!skipped) {
            skipped = true;
            continue;
        }
        row_string.set_zero();
        for (size_t column = 1; column < columns.size(); ++column) {
            const std::string &element(dsv_row[column - 1]);
            if (element.empty()) {
                row_string.set_null(columns[column].get_type(),
                                    columns[column].get_array_size(), column,
                                    offsets[column], offsets[column + 1]);
            } else {
                insert_ascii_in_row(columns[column].get_type(),
                                    columns[column].get_array_size(), column, element,
                                    offsets[column], offsets[column + 1], row_string);
            }
        }
        tablator::append_row(data, row_string);
    }
    return data;
}
