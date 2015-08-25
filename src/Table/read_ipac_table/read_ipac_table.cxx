#include <limits>
#include <vector>
#include <array>

#include <boost/lexical_cast.hpp>

#include "../../Table.hxx"

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
  char row_string[row_size];
  while (ipac_file)
    {
      clear_nulls (row_string);
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
              set_null (column, row_string);
            }
          else
            {
              switch (types[column])
                {
                  /// BOOLEAN and SHORT should never be used, but we include
                  /// them here just in case
                case Type::BOOLEAN:
                  try
                    {
                      int8_t result=stoi (element);
                      if (result !=0 || result !=1)
                        throw std::exception ();
                      copy_to_row (result, offsets[column], row_string);
                    }
                  catch (std::exception &error)
                    {
                      throw std::runtime_error ("Bad boolean in line "
                                                + std::to_string(current_line)
                                                + ", field "
                                                + std::to_string(column)
                                                + ".  Expected 0 or 1, but found '"
                                                + element + "'");
                    }
                  break;
                case Type::SHORT:
                  try
                    {
                      int result=boost::lexical_cast<int> (element);
                      if (result > std::numeric_limits<int16_t>::max ()
                          || result < std::numeric_limits<int16_t>::lowest ())
                        throw std::exception ();
                      copy_to_row (static_cast<int16_t> (result),
                                   offsets[column], row_string);
                    }
                  catch (std::exception &error)
                    {
                      throw std::runtime_error ("Bad short in line "
                                                + std::to_string(current_line)
                                                + ", field "
                                                + std::to_string(column)
                                                + ".  Found '"
                                                + element + "'");
                    }
                  break;
                case Type::INT:
                  try
                    {
                      long result=boost::lexical_cast<long> (element);
                      if (result > std::numeric_limits<int32_t>::max ()
                          || result < std::numeric_limits<int32_t>::lowest ())
                        throw std::exception ();
                      copy_to_row (static_cast<int32_t> (result),
                                   offsets[column], row_string);
                    }
                  catch (std::exception &error)
                    {
                      throw std::runtime_error ("Bad int in line "
                                                + std::to_string(current_line)
                                                + ", field "
                                                + std::to_string(column)
                                                + ".  Found '"
                                                + element + "'");
                    }
                  break;
                case Type::LONG:
                  try
                    {
                      long long result=boost::lexical_cast<long long> (element);
                      if (result > std::numeric_limits<int64_t>::max ()
                          || result < std::numeric_limits<int64_t>::lowest ())
                        throw std::exception ();
                      copy_to_row (static_cast<int64_t> (result),
                                   offsets[column], row_string);
                    }
                  catch (std::exception &error)
                    {
                      throw std::runtime_error ("Bad long in line "
                                                + std::to_string(current_line)
                                                + ", field "
                                                + std::to_string(column)
                                                + ".  Found '"
                                                + element + "'");
                    }
                  break;
                case Type::FLOAT:
                  try
                    {
                      float result=boost::lexical_cast<float> (element);
                      copy_to_row (result, offsets[column], row_string);
                    }
                  catch (std::exception &error)
                    {
                      throw std::runtime_error ("Bad float in line "
                                                + std::to_string(current_line)
                                                + ", field "
                                                + std::to_string(column)
                                                + ".  Found '"
                                                + element + "'");
                    }
                  break;
                case Type::DOUBLE:
                  try
                    {
                      double result=boost::lexical_cast<double> (element);
                      copy_to_row (result, offsets[column], row_string);
                    }
                  catch (std::exception &error)
                    {
                      throw std::runtime_error ("Bad double in line "
                                                + std::to_string(current_line)
                                                + ", field "
                                                + std::to_string(column)
                                                + ".  Found '"
                                                + element + "'");
                    }
                  break;
                case Type::STRING:
                  copy_to_row (element, offsets[column],
                               offsets[column+1], row_string);
                  break;
                }
            }
        }
      insert_row (row_string);
      ++current_line;
      std::getline (ipac_file,line);
    }
}
