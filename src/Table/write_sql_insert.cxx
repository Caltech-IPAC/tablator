#include "../Data_Type_to_SQL.hxx"
#include "../Table.hxx"
#include "../quote_sql_string.hxx"
#include "../write_type_as_ascii.hxx"

namespace {
void write_point(std::ostream &os,
                 const std::pair<std::pair<size_t, tablator::Data_Type>,
                                 std::pair<size_t, tablator::Data_Type> > &point_input,
                 const uint8_t *row_start) {
    os << "ST_MakePoint(";
    write_type_as_ascii(os, point_input.first.second, 1,
                        row_start + point_input.first.first);
    os << ", ";
    write_type_as_ascii(os, point_input.second.second, 1,
                        row_start + point_input.second.first);
    os << "),\n";
}

}  // namespace

void tablator::Table::write_sql_insert(
        std::ostream &os, const std::string &quoted_table_name,
        const size_t &row_offset, const bool &has_point,
        const std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type> >
                &point_input,
        const std::vector<
                std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type> > >
                &polygon_input) const {
    const auto &data = get_data();
    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    os << "INSERT INTO " << quoted_table_name << "\nVALUES (";
    if (has_point) {
        write_point(os, point_input, data.data() + row_offset);
    }
    for (auto &point : polygon_input) {
        write_point(os, point, data.data() + row_offset);
    }

    for (size_t column = 1; column < columns.size(); ++column) {
        if (is_null(row_offset, column)) {
            os << "NULL";
        } else {
            if (columns[column].get_type() == Data_Type::CHAR) {
                std::stringstream ss;
                write_type_as_ascii(ss, columns[column].get_type(),
                                    columns[column].get_array_size(),
                                    data.data() + row_offset + offsets[column], ' ');
                os << quote_sql_string(ss.str(), '\'');
            } else {
                if (columns[column].get_array_size() != 1) {
                    os << "'{";
                }
                write_type_as_ascii(os, columns[column].get_type(),
                                    columns[column].get_array_size(),
                                    data.data() + row_offset + offsets[column], ',');
                if (columns[column].get_array_size() != 1) {
                    os << "}'";
                }
            }
        }
        if (column + 1 != columns.size()) {
            os << ", ";
        } else {
            os << ");\n";
        }
    }
}
