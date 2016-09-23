/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Lesser General Public License as published by
/// the Free Software Foundation, version 3 of the License.

/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Lesser General Public License for more details.

/// You should have received a copy of the GNU Lesser General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// Copied from libcsv++
/// https://github.com/jainyzau/libcsv-

#pragma once

#include "../CSV_Document.hxx"
#include <fstream>

namespace CSV
{
  class CSV_Parser {
  public:
    enum ParseState{
      LineStart,
      FieldStart,
      FrontQuote,
      BackQuote,
      EscapeOn,
      EscapeOff,
      FieldEnd,
      LineEnd,
      ParseCompleted
    };
    CSV_Parser() = delete;
    CSV_Parser(CSV_Document* p_doc, const std::string& file_path);		

  private:
    inline char _curr_char() const;
    inline void _next();

    void _line_start();
    void _field_start();
    void _field_end();
    void _escape_on();
    void _escape_off();
    void _front_quote();
    void _back_quote();
    void _line_end();

    void _post_line_start();
    void _post_field_start();
    void _post_front_quote();
    void _post_escape_on();
    void _post_escape_off();
    void _post_back_quote();
    void _post_field_end();
    void _post_line_end();

    void _open_csv_file(const std::string& file_path);
    std::ifstream& _get_line_from_file();
    void _append_another_line_from_file();
    void _initialize(CSV_Document* p_doc, const std::string& file_path);

    std::string read_str;
    std::string row_str;
    CSV_Document::row_index_type row_count;
    CSV_Document::column_index_type col_count;
    std::string::size_type idx;
    std::string::size_type field_beg;
    std::string::size_type field_end;
    std::string elem;
    CSV_Document::row_type row;
    ParseState state;
    CSV_Document* document;
    std::ifstream csv_file;
  };
}
