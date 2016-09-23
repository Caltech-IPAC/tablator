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

#include <vector>
#include <list>
#include <string>
#include <fstream>
namespace CSV
{
  class CSVParser;

  class CSVDocument
  {
  public:
    typedef std::string element_type;
    typedef std::vector<element_type> row_type;
    typedef std::list<row_type> document_type;
    typedef document_type::size_type row_index_type;
    typedef row_type::size_type column_index_type;
    typedef document_type::iterator iterator;
    typedef document_type::const_iterator const_iterator;

    CSVDocument(const std::string& file_path);
    bool empty() const { return document.empty(); }
    iterator begin() { return document.begin(); }
    iterator end() { return document.end(); }
    const_iterator begin() const { return document.begin(); }
    const_iterator end() const { return document.end(); }

    void add_row(const row_type& row) { document.push_back(row); }

    document_type document;
  };

  class CSVParser {
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

    CSVParser();
    CSVDocument::row_index_type parse(CSVDocument* p_doc, const std::string& file_path);		

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
    void _initialize(CSVDocument* p_doc, const std::string& file_path);

    std::string read_str;
    std::string row_str;
    CSVDocument::row_index_type row_count;
    CSVDocument::column_index_type col_count;
    std::string::size_type idx;
    std::string::size_type field_beg;
    std::string::size_type field_end;
    std::string elem;
    CSVDocument::row_type row;
    ParseState state;
    CSVDocument* document;
    std::ifstream csv_file;
  };

}

