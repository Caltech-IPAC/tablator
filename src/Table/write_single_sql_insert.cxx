#include "../Ascii_Writer.hxx"
#include "../Data_Type_to_SQL.hxx"
#include "../Table.hxx"
#include "../quote_sql_string.hxx"

namespace {
void write_point(
        std::ostream &os,
        const std::pair<std::pair<size_t, tablator::Format_Packet>,
                        std::pair<size_t, tablator::Format_Packet> > &point_input,
        const uint8_t *row_start, const tablator::Command_Line_Options &options) {
    os << "ST_MakePoint(";
    size_t array_size = 1;
    tablator::Ascii_Writer::write_type_as_ascii(
            os, point_input.first.second, array_size,
            row_start + point_input.first.first,
            tablator::Ascii_Writer::DEFAULT_SEPARATOR, options);
    os << ", ";
    tablator::Ascii_Writer::write_type_as_ascii(
            os, point_input.second.second, array_size,
            row_start + point_input.second.first,
            tablator::Ascii_Writer::DEFAULT_SEPARATOR, options);
    os << "),\n";
}

}  // namespace

void tablator::Table::write_single_sql_insert(
        std::ostream &os, const std::string &quoted_table_name, const size_t &row_idx,
        const bool &has_point,
        const std::pair<std::pair<size_t, Format_Packet>,
                        std::pair<size_t, Format_Packet> > &point_input,
        const std::vector<std::pair<std::pair<size_t, Format_Packet>,
                                    std::pair<size_t, Format_Packet> > > &polygon_input,
        const Command_Line_Options &options) const {
    const auto &data = get_data();
    auto row_offset = row_idx * get_row_size();

    os << "INSERT INTO " << quoted_table_name << "\nVALUES (";
    if (has_point) {
        write_point(os, point_input, data.data() + row_offset, options);
    }
    for (auto &point : polygon_input) {
        write_point(os, point, data.data() + row_offset, options);
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
                        ss, column.get_format_packet(), column.get_array_size(),
                        data.data() + row_offset + offsets[col_idx], ' ', options);
                os << quote_sql_string(ss.str(), '\'');
            } else {
                if (column.get_array_size() != 1) {
                    os << "'{";
                }
                tablator::Ascii_Writer::write_type_as_ascii(
                        os, column.get_format_packet(), column.get_array_size(),
                        data.data() + row_offset + offsets[col_idx], ',', options);
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
