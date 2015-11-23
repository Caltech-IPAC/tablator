#include <limits>
#include <vector>
#include <array>

#include <boost/lexical_cast.hpp>

#include "../../Table.hxx"
#include "../../to_string.hxx"

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
      row_string.clear_nulls ();
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
              /// BOOLEAN and SHORT should never be used, but we include
              /// them here just in case
              try
                {
                  if (types[column]==H5::PredType::STD_I8LE)
                    {
                      int8_t result=stoi (element);
                      if (result !=0 || result !=1)
                        throw std::exception ();
                      row_string.insert (result, offsets[column]);
                    }
                  else if (types[column]==H5::PredType::STD_I16LE)
                    {
                      int result=boost::lexical_cast<int> (element);
                      if (result > std::numeric_limits<int16_t>::max ()
                          || result < std::numeric_limits<int16_t>::lowest ())
                        throw std::exception ();
                      row_string.insert (static_cast<int16_t> (result),
                                         offsets[column]);
                    }
                  else if (types[column]==H5::PredType::STD_I32LE)
                    {
                      long result=boost::lexical_cast<long> (element);
                      if (result > std::numeric_limits<int32_t>::max ()
                          || result < std::numeric_limits<int32_t>::lowest ())
                        throw std::exception ();
                      row_string.insert (static_cast<int32_t> (result),
                                         offsets[column]);
                    }
                  else if (types[column]==H5::PredType::STD_I64LE)
                    {
                      long long result=boost::lexical_cast<long long> (element);
                      if (result > std::numeric_limits<int64_t>::max ()
                          || result < std::numeric_limits<int64_t>::lowest ())
                        throw std::exception ();
                      row_string.insert (static_cast<int64_t> (result),
                                         offsets[column]);
                    }
                  else if (types[column]==H5::PredType::IEEE_F32LE)
                    {
                      float result=boost::lexical_cast<float> (element);
                      row_string.insert (result, offsets[column]);
                    }
                  else if (types[column]==H5::PredType::IEEE_F64LE)
                    {
                      double result=boost::lexical_cast<double> (element);
                      row_string.insert (result, offsets[column]);
                    }
                  else if (types[column]==H5::PredType::C_S1)
                    {
                      row_string.insert (element, offsets[column],
                                         offsets[column+1]);
                    }
                }
              catch (std::exception &error)
                {
                  throw std::runtime_error ("Bad "
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
