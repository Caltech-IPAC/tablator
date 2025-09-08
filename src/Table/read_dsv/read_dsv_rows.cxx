#include "../../Table.hxx"

namespace tablator {

// JTODO this function should not be a Table class member.
Data_Details Table::read_dsv_rows(Field_Framework &field_framework,
                                  const std::list<std::vector<std::string> > &dsv) {
    auto &columns = field_framework.get_columns();
    auto &offsets = field_framework.get_offsets();

    Data_Details data_details(field_framework, dsv.size());
    Row single_row(field_framework);

    bool top_row(true);
    for (auto &dsv_row : dsv) {
        if (top_row) {
            top_row = false;
            continue;
        }
        single_row.fill_with_zeros();
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            const std::string &element(dsv_row[col_idx - 1]);
            const auto &column = columns[col_idx];
            if (element.empty() || element == "null") {
                single_row.insert_null(column.get_type(), column.get_array_size(),
                                       offsets[col_idx], offsets[col_idx + 1], col_idx,
                                       column.get_dynamic_array_flag());
            } else {
                // The only variable-size array cols supported for DSV formats are of
                // type CHAR.
                uint32_t curr_array_size = (column.get_type() == Data_Type::CHAR)
                                                   ? element.size()
                                                   : column.get_array_size();
                single_row.insert_from_ascii(
                        element, column.get_type(), column.get_array_size(),
                        offsets[col_idx], offsets[col_idx + 1], col_idx,
                        curr_array_size, column.get_dynamic_array_flag());
            }
        }
        data_details.append_row(single_row);
    }
    return data_details;
}

}  // namespace tablator
