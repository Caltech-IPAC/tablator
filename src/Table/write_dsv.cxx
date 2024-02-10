// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "../Table.hxx"

#include <iomanip>

#include "../Common.hxx"
#include "../write_type_as_ascii.hxx"

namespace {
// Copied from libcsv++
// https://github.com/jainyzau/libcsv-

std::ostream &write_escaped_string(std::ostream &os, const std::string &s,
                                   const char &separator,
                                   const char &outer_quote_char) {
    auto quote_location(s.find(outer_quote_char));
    if (quote_location == std::string::npos &&
        (s.find(separator) != std::string::npos || s.find("\n") != std::string::npos)) {
        os << outer_quote_char << s << outer_quote_char;
    } else {
        os << s;
    }
    return os;
}
}  // namespace

void tablator::Table::write_dsv(std::ostream &os, const char &separator,
                                const Command_Line_Options &options) const {
    // Write comments.
    const auto &comments = get_comments();
    for (const auto comment : comments) {
        os << "# " << comment << "\n";
    }

    // Write body of table.
    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    const auto &data = get_data();
    const int num_columns = columns.size();
    if (num_columns == 0) return;
    // Skip null_bitfield_flags
    for (int i = 1; i < num_columns; ++i) {
        write_escaped_string(os, columns[i].get_name(), separator,
                             tablator::DOUBLE_QUOTE);
        os << (i == num_columns - 1 ? '\n' : separator);
    }

    for (size_t row_offset = 0; row_offset < data.size(); row_offset += row_size())
        for (int i = 1; i < num_columns; ++i) {
            size_t offset = offsets[i] + row_offset;
            std::stringstream ss;
            if (!is_null(row_offset, i)) {
                write_type_as_ascii(ss, columns[i].get_type(),
                                    columns[i].get_array_size(), data.data() + offset);
            } else if (options.write_null_strings_) {
                // Leave null entries blank by default, unlike in IPAC_TABLE format.
                ss << "null";
            }
            char outer_quote_char = '\"';
            if (!ss.str().empty() && ss.str()[0] == '\'') {
                outer_quote_char = '\'';
            }
            write_escaped_string(os, ss.str(), separator, outer_quote_char);
            os << (i == num_columns - 1 ? '\n' : separator);
        }
}
