#include "../../Table.hxx"

namespace tablator {

// JTODO this function should not be a Table class member.
Data_Details Table::read_dsv_rows(Field_Framework &field_framework,
                                  const std::list<std::vector<std::string> > &dsv) {
    auto &columns = field_framework.get_columns();
    auto &offsets = field_framework.get_offsets();

    Data_Details data_details(field_framework, dsv.size());

    Row single_row(field_framework);

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
        data_details.append_row(single_row);
    }
    return data_details;
}

}  // namespace tablator
