// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Originally copied from libcsv++
// https://github.com/jainyzau/libcsv-

// Extensively modified by Walter Landry

#include "DSV_Parser.hxx"

#include <sstream>
#include <stdexcept>

#include "../../../Common.hxx"

namespace tablator {

// NOTE: This parser expects any DOUBLE_QUOTE character (") inside a
// string which is itself enclosed in DOUBLE_QUOTE characters to be
// represented by two consecutive DOUBLE_QUOTE characters (""), and
// similarly for SINGLE_QUOTE characters inside SINGLE_QUOTEd strings.
// Much of the complexity below stems from the necessity to establish
// whether a XXX_QUOTE character encountered in a loop through an
// XXX_QUOTEd string represents the end of the string or rather serves
// as an escape character for the XXX_QUOTE character immediately
// following it.

DSV_Parser::DSV_Parser(DSV_Document &p_doc, std::istream &input_stream,
                       const char &Delimiter)
        : document(p_doc), dsv_stream(input_stream), delimiter(Delimiter) {
    idx = field_beg = field_end = 0;
    row_count = col_count = 0;
    state = LineEnd;
    _initialize();
    for (; state != ParseCompleted; _next()) {
        // I know design pattern, but I don't want to make things more
        // complicated.
        switch (state) {
            case LineStart:
                _post_line_start();
                break;
            case FieldStart:
                _post_field_start();
                break;
            case FrontQuote:
                _post_front_quote();
                break;
            case EscapeOn:
                _post_escape_on();
                break;
            case EscapeOff:
                _post_escape_off();
                break;
            case BackQuote:
                _post_back_quote();
                break;
            case FieldEnd:
                _post_field_end();
                break;
            case LineEnd:
                _post_line_end();
                break;
            case ParseCompleted:
                // Silence a warning about unhandled case.
                break;
        }
    }
}

char DSV_Parser::_curr_char() const { return row_str[idx]; }

void DSV_Parser::_next() { ++idx; }

void DSV_Parser::_post_line_start() {}

void DSV_Parser::_post_field_start() {
    if (_curr_char() == delimiter) {
        field_end = idx;
        _field_end();
    } else if (_curr_char() == '\n') {
        field_end = idx;
        _field_end();
        _line_end();
    }
}

void DSV_Parser::_post_front_quote() {
    if (_curr_char() == outer_quote_char) {
        _escape_on();
    } else if (_curr_char() == '\n') {
        _append_another_line_from_file();
    }
}

void DSV_Parser::_post_escape_on() {
    if (_curr_char() == delimiter) {
        _back_quote();
        _field_end();
    } else if (_curr_char() == outer_quote_char) {
        _escape_off();
    } else if (_curr_char() == '\n') {
        _back_quote();
        _field_end();
        _line_end();
    }
}

void DSV_Parser::_post_escape_off() {
    if (_curr_char() == outer_quote_char) {
        _escape_on();
    } else if (_curr_char() == '\n') {
        _append_another_line_from_file();
    }
}

void DSV_Parser::_post_back_quote() {}

void DSV_Parser::_post_field_end() {
    _field_start();

    if (_curr_char() == DOUBLE_QUOTE || _curr_char() == SINGLE_QUOTE) {
        outer_quote_char = _curr_char();
        _front_quote();
    } else if (_curr_char() == delimiter) {
        _field_end();
    } else if (_curr_char() == '\n') {
        _field_end();
        _line_end();
    }
}

void DSV_Parser::_post_line_end() {
    _line_start();
    _field_start();

    if (_curr_char() == DOUBLE_QUOTE || _curr_char() == SINGLE_QUOTE) {
        outer_quote_char = _curr_char();
        _front_quote();
    } else if (_curr_char() == delimiter) {
        _field_end();
    } else if (_curr_char() == '\n') {
        throw std::runtime_error("Syntax error: empty line is not allowed.");
    }
}

bool DSV_Parser::_get_line_from_file() {
    if (std::getline(dsv_stream, read_str)) {
        if (read_str.size() > 0 && *read_str.rbegin() == '\r') {
            read_str = read_str.substr(0, read_str.size() - 1);
        }
        read_str += '\n';
        return true;
    } else if (dsv_stream.eof()) {
        state = ParseCompleted;
    } else {
        throw std::runtime_error("Internal error: failed to read more data from file.");
    }

    return dsv_stream.good();
}

void DSV_Parser::_append_another_line_from_file() {
    if (_get_line_from_file()) {
        row_str.erase(0, field_beg);
        row_str += read_str;
        idx -= field_beg;
        field_beg = field_end = 0;
    } else if (dsv_stream.eof()) {
        throw std::runtime_error("No more data in file. Parsing is not completed.");
    }
}

void DSV_Parser::_initialize() { _line_end(); }

void DSV_Parser::_field_end() {
    elem.append(row_str.c_str() + field_beg, field_end - field_beg);
    field_beg = idx + 1;
    row.push_back(elem);
    state = FieldEnd;
}

void DSV_Parser::_line_start() {
    row.clear();
    ++row_count;
    idx = 0;
    state = LineStart;
}

void DSV_Parser::_field_start() {
    elem.clear();
    field_beg = field_end = idx;
    state = FieldStart;
    outer_quote_char = NULL_CHAR;
}

void DSV_Parser::_escape_on() { state = EscapeOn; }

void DSV_Parser::_escape_off() {
    elem.append(row_str.c_str() + field_beg, idx + 1 - field_beg);
    field_beg = idx + 1;
    state = EscapeOff;
}

void DSV_Parser::_front_quote() { state = FrontQuote; }

void DSV_Parser::_back_quote() {
    field_end = idx;

    // Note: FrontQuote (not BackQuote) is in the original libcsv++.
    state = FrontQuote;
}

void DSV_Parser::_line_end() {
    if (row.size() > 0) {
        if (col_count == 0) {
            col_count = row.size();  // Initialize col_count to the size of the
                                     // first valid line.
        }

        if (row.size() != col_count) {
            std::ostringstream str_stream;
            str_stream << "Syntax error: too "
                       << (row.size() > col_count ? "many" : "few")
                       << " fields in line " << row_count << ".";
            throw std::runtime_error(str_stream.str());
        } else {
            document.push_back(row);
        }
    } else if (col_count > 0) {
        throw std::runtime_error("Syntax error: empty line is not allowed.");
    }

    if (_get_line_from_file()) {
        row_str = read_str;
        state = LineEnd;
    }
}
}  // namespace tablator
