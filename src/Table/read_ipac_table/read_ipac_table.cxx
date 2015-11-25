#include <limits>
#include <vector>
#include <array>

#include <boost/lexical_cast.hpp>

#include "../../Table.hxx"
#include "../../to_string.hxx"
#include "../insert_ascii_in_row.hxx"

void tablator::Table::read_ipac_table (const boost::filesystem::path &path)
{
  if (!exists (path))
    throw std::runtime_error ("File " + path.string () + "does not exist.");

  boost::filesystem::ifstream ipac_file (path, std::ios::in);

  size_t current_line;
  std::array<std::vector<std::string>,4> columns;
  std::vector<size_t> ipac_column_offsets, ipac_column_widths;

  current_line=read_ipac_header (ipac_file,columns,ipac_column_offsets);
  create_types_from_ipac_headers (columns,ipac_column_offsets,
                                  ipac_column_widths);

  std::string line;
  std::getline (ipac_file,line);
  Row row_string (row_size);
  while (ipac_file)
    {
      row_string.set_zero ();
      for (size_t column=1; column < columns[0].size (); ++column)
        {
          if (line[ipac_column_offsets[column-1]]!=' ')
            throw std::runtime_error ("Non-space found at a delimiter location on line "
                                      + std::to_string(current_line)
                                      + ", column "
                                      + std::to_string (ipac_column_offsets[column-1])
                                      + ".  Is a field not wide enough?");

          std::string element=line.substr (ipac_column_offsets[column-1]+1,
                                           ipac_column_widths[column]);
          boost::algorithm::trim(element);
          if (!columns[3][column].empty()
              && element==columns[3][column])
            {
              row_string.set_null (column, types[column], offsets);
            }
          else
            {
              try
                {
                  insert_ascii_in_row (types[column], element, column, offsets,
                                       row_string);
                }
              catch (std::exception &error)
                {
                  throw std::runtime_error ("Invalid "
                                            + to_string (types[column])
                                            + " in line "
                                            + std::to_string(current_line)
                                            + ", field "
                                            + std::to_string(column)
                                            + ".  Found '"
                                            + element + "'");
                }
            }
        }
      append_row (row_string);
      ++current_line;
      std::getline (ipac_file,line);
    }
}
