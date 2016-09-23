/// This program is free software: you can redistribute it and/or modify
/// it under the terms of the GNU Lesser General Public License as published by
/// the Free Software Foundation, version 3 of the License.

/// This program is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU Lesser General Public License for more details.

/// You should have received a copy of the GNU Lesser General Public License
/// along with this program.  If not, see <http://www.gnu.org/licenses/>.

/// Originally copied from libcsv++
/// https://github.com/jainyzau/libcsv-

/// Exensively modified by Walter Landry

#pragma once

#include <fstream>
#include <list>
#include <vector>
#include <string>
namespace tablator
{
  typedef std::list<std::vector<std::string> > DSV_Document;
  class DSV_Parser {
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
    DSV_Parser() = delete;
    DSV_Parser(DSV_Document &p_doc, const std::string& file_path);		

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

    void _open_dsv_file(const std::string& file_path);
    std::ifstream& _get_line_from_file();
    void _append_another_line_from_file();
    void _initialize(const std::string& file_path);

    std::string read_str;
    std::string row_str;
    size_t row_count;
    size_t col_count;
    std::string::size_type idx;
    std::string::size_type field_beg;
    std::string::size_type field_end;
    std::string elem;
    std::vector<std::string> row;
    ParseState state;
    DSV_Document &document;
    std::ifstream dsv_file;
  };
}
