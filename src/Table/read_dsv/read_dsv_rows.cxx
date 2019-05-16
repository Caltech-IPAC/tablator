#include "../../Table.hxx"
#include "../insert_ascii_in_row.hxx"

void tablator::Table::read_dsv_rows(const std::list<std::vector<std::string> > &dsv) {
    bool skipped(false);
    Row row_string(row_size());
    for (auto &dsv_row : dsv) {
        if (!skipped) {
            skipped = true;
            continue;
        }
        row_string.set_zero();
        for (size_t column = 1; column < columns.size(); ++column) {
            const std::string &element(dsv_row[column - 1]);
            if (element.empty()) {
                row_string.set_null(columns[column].type, columns[column].array_size,
                                    column, offsets[column], offsets[column + 1]);
            } else {
                insert_ascii_in_row(columns[column].type, columns[column].array_size,
                                    column, element, offsets[column],
                                    offsets[column + 1], row_string);
            }
        }
        append_row(row_string);
    }
}
