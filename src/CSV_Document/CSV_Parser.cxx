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

#include "CSV_Parser.hxx"

#include <sstream>
#include <stdexcept>
namespace tablator
{
  CSV_Parser::CSV_Parser(CSV_Document &p_doc, const std::string& file_path): document (p_doc)
  {
    idx = field_beg = field_end = 0;
    row_count = col_count = 0;
    state = LineEnd;
    _initialize(file_path);
    for (; state != ParseCompleted; _next())
      {
        // I know design pattern, but I don't want to make things more complicated.
        switch(state)
          {
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
            /// Silence a warning about unhandled case.
            break;
          }
      }
  }

  char CSV_Parser::_curr_char() const
  {
    return row_str[idx];
  }

  void CSV_Parser::_next()
  {
    ++idx;
  }

  void CSV_Parser::_post_line_start()
  {
  }

  void CSV_Parser::_post_field_start()
  {
    if (_curr_char() == ',')
      {
        field_end = idx;
        _field_end();
      }
    else if (_curr_char() == '\n')
      {
        field_end = idx;
        _field_end();
        _line_end();
      }
    else if (_curr_char() == '\"')
      {
        if (field_beg != idx)
          {
            throw std::runtime_error("Syntax error: quote symbol in unenclosed field.");
          }
      }
  }

  void CSV_Parser::_post_front_quote()
  {
    if (_curr_char() == '"')
      {
        _escape_on();
      }
    else if (_curr_char() == '\n')
      {
        _append_another_line_from_file();
      }
  }

  void CSV_Parser::_post_escape_on()
  {
    if (_curr_char() == ',')
      {
        _back_quote();
        _field_end();
      }
    else if (_curr_char() == '"')
      {
        _escape_off();
      }
    else if (_curr_char() == '\n')
      {
        _back_quote();
        _field_end();
        _line_end();
      }
    else
      {
        throw std::runtime_error("Syntax error: quote symbol in front of character which is not quote symbol.");
      }
  }

  void CSV_Parser::_post_escape_off()
  {
    if (_curr_char() == '"')
      {
        _escape_on();
      }
    else if (_curr_char() == '\n')
      {
        _append_another_line_from_file();
      }
  }

  void CSV_Parser::_post_back_quote()
  {
  }

  void CSV_Parser::_post_field_end()
  {
    _field_start();

    if (_curr_char() == '"')
      {
        _front_quote();
      }
    else if (_curr_char() == ',')
      {
        _field_end();
      }
    else if (_curr_char() == '\n')
      {
        _field_end();
        _line_end();
      }
  }

  void CSV_Parser::_post_line_end()
  {
    _line_start();
    _field_start();

    if (_curr_char() == '"')
      {
        _front_quote();
      }
    else if (_curr_char() == ',')
      {
        _field_end();
      }
    else if (_curr_char() == '\n')
      {
        throw std::runtime_error("Syntax error: empty line is not allowed.");
      }
  }

  void CSV_Parser::_open_csv_file( const std::string& file_path )
  {
    csv_file.open(file_path.c_str());
    if (csv_file.fail())
      {
        throw std::runtime_error("Failed to open file " + file_path + ".");
      }
  }

  std::ifstream& CSV_Parser::_get_line_from_file()
  {
    if (std::getline(csv_file, read_str))
      {
        read_str += '\n';
      }
    else if (csv_file.eof())
      {
        state = ParseCompleted;
      }
    else
      {
        throw std::runtime_error("Internal error: failed to read more data from file.");
      }

    return csv_file;
  }

  void CSV_Parser::_append_another_line_from_file()
  {
    if (_get_line_from_file())
      {
        row_str.erase(0, field_beg);
        row_str += read_str;
        idx -= field_beg;
        field_beg = field_end = 0;
      }
    else if (csv_file.eof())
      {
        throw std::runtime_error("No more data in file. Parsing is not completed.");
      }
  }

  void CSV_Parser::_initialize(const std::string& file_path)
  {
    _open_csv_file(file_path);
    _line_end();
  }

  void CSV_Parser::_field_end()
  {
    elem.append(row_str.c_str() + field_beg, field_end - field_beg);
    field_beg = idx + 1;
    row.push_back(elem);
    state = FieldEnd;
  }

  void CSV_Parser::_line_start()
  {
    row.clear();
    ++row_count;
    idx = 0;
    state = LineStart;
  }

  void CSV_Parser::_field_start()
  {
    elem.clear();
    field_beg = field_end = idx;
    state = FieldStart;
  }

  void CSV_Parser::_escape_on()
  {
    state = EscapeOn;
  }

  void CSV_Parser::_escape_off()
  {
    elem.append(row_str.c_str() + field_beg, idx - field_beg);
    field_beg = idx + 1;
    state = EscapeOff;
  }

  void CSV_Parser::_front_quote()
  {
    ++field_beg;
    state = FrontQuote;
  }

  void CSV_Parser::_back_quote()
  {
    field_end = idx - 1;
    state = FrontQuote;
  }

  void CSV_Parser::_line_end()
  {
    if (row.size() > 0)
      {
        if (col_count == 0)
          {
            col_count = row.size(); // Initialize col_count to the size of the first valid line.
          }

        if (row.size() != col_count)
          {
            std::ostringstream str_stream;
            str_stream << "Syntax error: too " 
                       << (row.size() > col_count ? "much" : "few") 
                       << " fields in line " << row_count << ".";
            throw std::runtime_error(str_stream.str());
          }
        else
          {
            document.add_row(row);
          }
      }
    else if(col_count > 0)
      {
        throw std::runtime_error("Syntax error: empty line is not allowed.");
      }

    if (_get_line_from_file())
      {
        row_str = read_str;
        state = LineEnd;
      }
  }

}
