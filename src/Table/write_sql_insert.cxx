#include "../Ascii_Writer.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../Table.hxx"
#include "../quote_sql_string.hxx"

namespace {
void write_point(std::ostream &os,
                 const std::pair<std::pair<size_t, tablator::Data_Type>,
                                 std::pair<size_t, tablator::Data_Type> > &point_input,
                 const char *row_start,
                 const tablator::Command_Line_Options &options) {
    os << "ST_MakePoint(";
    tablator::Ascii_Writer::write_type_as_ascii(
            os, point_input.first.second, 1, row_start + point_input.first.first,
            tablator::Ascii_Writer::DEFAULT_SEPARATOR, options);
    os << ", ";
    tablator::Ascii_Writer::write_type_as_ascii(
            os, point_input.second.second, 1, row_start + point_input.second.first,
            tablator::Ascii_Writer::DEFAULT_SEPARATOR, options);
    os << "),\n";
}

}  // namespace

void tablator::Table::write_sql_insert(
        std::ostream &os, const std::string &quoted_table_name, const size_t &row_idx,
        const bool &has_point,
        const std::pair<std::pair<size_t, Data_Type>, std::pair<size_t, Data_Type> >
                &point_input,
        const std::vector<std::pair<std::pair<size_t, Data_Type>,
                                    std::pair<size_t, Data_Type> > > &polygon_input,
        const Command_Line_Options &options) const {
#if 0
    const auto &data = get_data();
    auto row_offset = row_idx * get_row_size();
#else
	const auto &curr_row_data = get_data().at(row_idx).data();
#endif
    os << "INSERT INTO " << quoted_table_name << "\nVALUES (";
    if (has_point) {
#if 0
        write_point(os, point_input, data.data() + row_offset, options);
#else
        write_point(os, point_input, curr_row_data, options);
#endif
    }
    for (auto &point : polygon_input) {
#if 0
        write_point(os, point, data.data() + row_offset, options);
#else
        write_point(os, point, curr_row_data, options);
#endif
    }

    const auto &columns = get_columns();
    const auto &offsets = get_offsets();

    for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
        if (is_null_value(row_idx, col_idx)) {
            os << "NULL";
        } else {
            const auto &column = columns[col_idx];
            if (column.get_type() == Data_Type::CHAR) {
                std::stringstream ss;
                tablator::Ascii_Writer::write_type_as_ascii(
                        ss, column.get_type(), column.get_array_size(),
#if 0
                        data.data() + row_offset + offsets[col_idx], ' ',
#else
						curr_row_data + offsets[col_idx], ' ',
#endif
						options);
                os << quote_sql_string(ss.str(), '\'');
            } else {
                if (column.get_array_size() != 1) {
                    os << "'{";
                }
                tablator::Ascii_Writer::write_type_as_ascii(
                        os, column.get_type(), column.get_array_size(),
#if 0
                        data.data() + row_offset + offsets[col_idx], ',',
#else
                        curr_row_data + offsets[col_idx], ',',
#endif
						options);
                if (column.get_array_size() != 1) {
                    os << "}'";
                }
            }
        }
        if (col_idx + 1 != columns.size()) {
            os << ", ";
        } else {
            os << ");\n";
        }
    }
}
