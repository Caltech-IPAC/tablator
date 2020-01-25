#include "../../Table.hxx"

namespace tablator {
std::list<std::vector<std::string> > parse_dsv(std::istream &input_stream,
                                               const char &delimiter);

void Table::read_dsv(std::istream &input_stream, const Format &format) {
    std::list<std::vector<std::string> > dsv(
            parse_dsv(input_stream, format.is_csv() ? ',' : '\t'));

    std::vector<Column> &columns = get_columns();
    std::vector<size_t> &offsets = get_offsets();
    set_column_info(columns, offsets, dsv);

    set_data(read_dsv_rows(columns, offsets, dsv));
}
}  // namespace tablator
