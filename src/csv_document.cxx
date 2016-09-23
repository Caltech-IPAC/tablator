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

#include "csv_document.hxx"
#include <fstream>
#include <sstream>
#include <stdexcept>
namespace CSV
{

	CSVDocument::row_index_type CSVDocument::load_file( const std::string& file_path )
	{
		// why use a separate parser? 
		// Because parser uses some local variables, if we use a separate parser, all these 
		// variables are de-constructed after loading while parsed data is still valid.
		CSVParser parser;
		return parser.parse(this, file_path);
	}

	void CSVDocument::add_row( const row_type& row )
	{
		document.push_back(row);
	}

	bool CSVDocument::empty() const
	{
		return document.empty();
	}

	CSVDocument::iterator CSVDocument::begin()
	{
		return document.begin();
	}

	CSVDocument::iterator CSVDocument::end()
	{
		return document.end();
	}

	CSVDocument::const_iterator CSVDocument::begin() const
	{
		return document.begin();
	}

	CSVDocument::const_iterator CSVDocument::end() const
	{
		return document.end();
	}

	void CSVDocument::_check_row_index( row_index_type row_idx ) const
	{
		if (row_idx >= document.size())
		{
			std::ostringstream str_stream;
			str_stream << "row index " << row_idx << " is out of range. (max: " << document.size() - 1 << ")";
			throw std::out_of_range(str_stream.str());
		}
	}

	void CSVDocument::_check_col_index( column_index_type col ) const
	{
		if (document.front().size() <= col)
		{
			std::ostringstream str_stream;
			str_stream << "column index " << col << " is out of range. (max: " << document.front().size() - 1 << ")";
			throw std::out_of_range(str_stream.str());
		}
	}

	CSVParser::CSVParser()
	{
		idx = field_beg = field_end = 0;
		row_count = col_count = 0;
		state = LineEnd;
	}

	CSVDocument::row_index_type CSVParser::parse( CSVDocument* p_doc, const std::string& file_path )
	{
		_initialize(p_doc, file_path);
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

		return row_count;
	}

	char CSVParser::_curr_char() const
	{
		return row_str[idx];
	}

	void CSVParser::_next()
	{
		++idx;
	}

	void CSVParser::_post_line_start()
	{
	}

	void CSVParser::_post_field_start()
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

	void CSVParser::_post_front_quote()
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

	void CSVParser::_post_escape_on()
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

	void CSVParser::_post_escape_off()
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

	void CSVParser::_post_back_quote()
	{
	}

	void CSVParser::_post_field_end()
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

	void CSVParser::_post_line_end()
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

	void CSVParser::_open_csv_file( const std::string& file_path )
	{
		csv_file.open(file_path.c_str());
		if (csv_file.fail())
		{
			throw std::runtime_error("Failed to open file " + file_path + ".");
		}
	}

	std::ifstream& CSVParser::_get_line_from_file()
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

	void CSVParser::_append_another_line_from_file()
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

	void CSVParser::_initialize( CSVDocument* p_doc, const std::string& file_path )
	{
		if (!p_doc)
		{
			throw std::runtime_error("Destination of parsed data is not set.");
		}
		document = p_doc;

		_open_csv_file(file_path);
		_line_end();
	}

	void CSVParser::_field_end()
	{
		elem.append(row_str.c_str() + field_beg, field_end - field_beg);
		field_beg = idx + 1;
		row.push_back(elem);
		state = FieldEnd;
	}

	void CSVParser::_line_start()
	{
		row.clear();
		++row_count;
		idx = 0;
		state = LineStart;
	}

	void CSVParser::_field_start()
	{
		elem.clear();
		field_beg = field_end = idx;
		state = FieldStart;
	}

	void CSVParser::_escape_on()
	{
		state = EscapeOn;
	}

	void CSVParser::_escape_off()
	{
		elem.append(row_str.c_str() + field_beg, idx - field_beg);
		field_beg = idx + 1;
		state = EscapeOff;
	}

	void CSVParser::_front_quote()
	{
		++field_beg;
		state = FrontQuote;
	}

	void CSVParser::_back_quote()
	{
		field_end = idx - 1;
		state = FrontQuote;
	}

	void CSVParser::_line_end()
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
				document->add_row(row);
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
