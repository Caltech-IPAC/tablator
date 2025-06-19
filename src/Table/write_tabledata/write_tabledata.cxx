#include <cmath>
#include <iomanip>
#include <limits>

#include "../../Ascii_Writer.hxx"
#include "../../Table.hxx"
#include "../../to_string.hxx"

namespace tablator {
std::string decode_links(const std::string &encoded);

void Table::splice_tabledata_and_write(std::ostream &os, std::stringstream &ss,
                                       Format::Enums enum_format, uint num_spaces_left,
                                       uint num_spaces_right,
                                       const Command_Line_Options &options) const {
    std::string s(ss.str());
    size_t tabledata_offset(s.find(TABLEDATA_PLACEHOLDER));
    os << s.substr(0, tabledata_offset - num_spaces_left);

    write_tabledata(os, enum_format, options);
    os << s.substr(tabledata_offset + TABLEDATA_PLACEHOLDER.size() + num_spaces_right);
}

void Table::write_tabledata(std::ostream &os, const Format::Enums &output_format,
                            const Command_Line_Options &options) const {
    std::string tr_prefix, tr_suffix, td_prefix, td_suffix;
    std::string tabledata_indent = "                    ";
    const bool is_json(output_format == Format::Enums::JSON ||
                       output_format == Format::Enums::JSON5);
    if (is_json) {
        tabledata_indent = "                    ";
        std::string tr_indent = tabledata_indent + "    ";
        tr_prefix = tr_indent + "[\n";
        tr_suffix = tr_indent + "]";

        std::string td_indent = tr_indent + "    ";
        td_prefix = td_indent + "\"";
        td_suffix = "\"";

        os << '\n' << tabledata_indent << "[\n";
    } else {
        tabledata_indent = "        ";
        std::string tr_indent = tabledata_indent + "  ";
        tr_prefix = tr_indent + "<TR>\n";
        tr_suffix = tr_indent + "</TR>";

        std::string td_indent = tr_indent + "  ";
        td_prefix = td_indent + "<TD>";
        td_suffix = "</TD>";
        os << '\n';
    }

    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    size_t num_rows = get_num_rows();
    const auto &data = get_data();

    for (size_t row_idx = 0, row_offset = 0; row_idx < num_rows;
         ++row_idx, row_offset += get_row_size()) {
        os << tr_prefix;

        // Skip the null bitfield flag
        for (size_t col_idx = 1; col_idx < columns.size(); ++col_idx) {
            const auto &column = columns[col_idx];
            std::stringstream td;
            // Leave null entries blank, unlike in IPAC_TABLE format.
            if (!is_null_value(row_idx, col_idx)) {
                Ascii_Writer::write_type_as_ascii(
                        td, column.get_type(), column.get_array_size(),
                        data.data() + row_offset + offsets[col_idx],
                        Ascii_Writer::DEFAULT_SEPARATOR, options);
            }
            os << td_prefix;
            switch (output_format) {
                case Format::Enums::JSON:
                case Format::Enums::JSON5:
                    // FIXME: This uses the undocumented character escapes.
                    os << boost::property_tree::json_parser::create_escapes(td.str());
                    break;
                case Format::Enums::HTML: {
                    os << decode_links(
                            boost::property_tree::xml_parser::encode_char_entities(
                                    td.str()));
                } break;
                default:
                    os << boost::property_tree::xml_parser::encode_char_entities(
                            td.str());
                    break;
            }
            os << td_suffix;
            if (is_json && col_idx < columns.size() - 1) {
                os << ',';
            }
            os << '\n';
        }
        os << tr_suffix;
        if (is_json && row_offset < data.size() - row_size()) {
            os << ',';
        }
        os << '\n';
    }
    os << tabledata_indent;
    if (is_json) {
        os << "]\n";
    }
}
}  // namespace tablator
