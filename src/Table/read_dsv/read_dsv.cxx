#include "../../Table.hxx"

namespace tablator {
std::list<std::vector<std::string> > parse_dsv(std::istream &input_stream,
                                               const char &delimiter);

void Table::read_dsv(std::istream &input_stream, const Format &format) {
    std::list<std::vector<std::string> > dsv(
            parse_dsv(input_stream, format.is_csv() ? ',' : '\t'));

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};
    set_column_info(columns, offsets, dsv);

    add_resource_element(Table_Element::Builder(columns, offsets,
                                                read_dsv_rows(columns, offsets, dsv))
                                 .build());
}
}  // namespace tablator
