/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Lesser General Public License as published by
/// the Free Software Foundation, version 3 of the License.

/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Lesser General Public License for more details.

/// You should have received a copy of the GNU Lesser General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iomanip>

#include "../Table.hxx"
#include "../write_type_as_ascii.hxx"

namespace {
/// Copied from libcsv++
/// https://github.com/jainyzau/libcsv-
std::ostream &write_escaped_string(std::ostream &os, const std::string &s,
                                   const char &separator) {
    auto quote_location(s.find("\""));
    if (quote_location != std::string::npos) {
        size_t start(0);
        os << "\"";
        while (quote_location != std::string::npos) {
            os << s.substr(start, quote_location - start) << "\"\"";
            start = quote_location + 1;
            quote_location = s.find("\"", start);
        }
        os << s.substr(start, quote_location - start) << "\"";
    } else if (s.find(separator) != std::string::npos ||
               s.find("\n") != std::string::npos) {
        os << "\"" << s << "\"";
    } else {
        os << s;
    }
    return os;
}
}  // namespace

void tablator::Table::write_dsv(std::ostream &os, const char &separator) const {
    const auto &columns = get_columns();
    const auto &offsets = get_offsets();
    const auto &data = get_data();
    const int num_members = columns.size();
    if (num_members == 0) return;
    /// Skip null_bitfield_flags
    for (int i = 1; i < num_members; ++i) {
        write_escaped_string(os, columns[i].get_name(), separator);
        os << (i == num_members - 1 ? '\n' : separator);
    }

    for (size_t row_offset = 0; row_offset < data.size(); row_offset += row_size())
        for (int i = 1; i < num_members; ++i) {
            size_t offset = offsets[i] + row_offset;
            std::stringstream ss;
            if (!is_null(row_offset, i)) {
                // Leave null entries blank, unlike in IPAC_TABLE format.
                write_type_as_ascii(ss, columns[i].get_type(),
                                    columns[i].get_array_size(), data.data() + offset);
            }
            write_escaped_string(os, ss.str(), separator);
            os << (i == num_members - 1 ? '\n' : separator);
        }
}
