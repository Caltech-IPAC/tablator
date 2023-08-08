#include "../../Table.hxx"

namespace tablator {
std::list<std::vector<std::string> > parse_dsv(std::istream &input_stream,
                                               const char &delimiter);

void Table::read_dsv(std::istream &input_stream, const Format &format) {
    // Read and load comments, skipping blank lines and empty comments.

    bool checking_for_comments = true;
    std::string line;

    while (checking_for_comments) {
        char first_character = input_stream.get();
        for (; input_stream && first_character == ' ';
             first_character = input_stream.get()) {
        };

        if (first_character == '#') {
            std::getline(input_stream, line);
            boost::algorithm::trim(line);
            if (!line.empty()) {
                add_comment(line);
            }

        } else if (first_character != '\n') {
            checking_for_comments = false;
            input_stream.putback(first_character);
        }
    }

    // Read body of table.

    std::list<std::vector<std::string> > dsv(
            parse_dsv(input_stream, format.is_csv() ? ',' : '\t'));

    // Construct main resource element from table data.

    std::vector<Column> columns;
    std::vector<size_t> offsets = {0};
    set_column_info(columns, offsets, dsv);
    add_resource_element(Table_Element::Builder(columns, offsets,
                                                read_dsv_rows(columns, offsets, dsv))
                                 .build());
}
}  // namespace tablator
