#include <boost/regex.hpp>

#include "../../Table.hxx"

namespace {

std::vector<size_t> get_bar_offsets (std::string &str)
{
  std::vector<size_t> offsets;
  for (size_t i = 0; i < str.size (); i++)
    if (str[i] == '|')
      offsets.push_back (i);
  return offsets;
}

void check_bar_position (std::vector<size_t> &bar_offsets, std::string &line,
                         const size_t &current_line)
{
  for (auto &offset: bar_offsets)
    if (line.size () <= offset || line.at (offset) != '|')
      throw std::runtime_error ("In line " + std::to_string (current_line)
                                + ", the bar '|' is misaligned");
}
}

size_t Tablator::Table::read_ipac_header
(boost::filesystem::ifstream &ipac_file,
 std::array<std::vector<std::string>,4> &columns,
 std::vector<size_t> &ipac_column_offsets)
{
  size_t current_line=0;
  char first_character=ipac_file.peek ();
  while (ipac_file && first_character=='\\')
    {
      std::string line;
      std::getline (ipac_file,line);
      ++current_line;
      if (line.size ()==1)
        continue;
      if (line[1]==' ')
        {
          comment.push_back (line.substr (2));
        }
      else
        {
          auto position_of_equal = line.find ("=");
          std::string key =
            boost::algorithm::trim_copy (line.substr (1, position_of_equal - 1));
          std::string value =
            boost::algorithm::trim_copy (line.substr (position_of_equal + 1));

          if (!boost::iequals (key, "fixlen"))
            {
              std::size_t first = value.find_first_not_of ("\"'");
              std::size_t last = value.find_last_not_of ("\"'");
              std::string value_substr=value.substr (first, last - first + 1);
              /// Handle duplicate keys by appending them to the end.
              auto p=properties.find (key);
              if (p!=properties.end ())
                {
                  p->second.value+=" " + value_substr;
                }
              else
                {
                  properties.insert (std::make_pair (key, Property (value_substr)));
                }
            }
        }
      first_character=ipac_file.peek ();
    }
  size_t column_line=0;
  while (ipac_file && first_character=='|')
    {
      std::string line;
      std::getline (ipac_file,line);
      ++current_line;
      auto tab_position=line.find ("\t");
      if (tab_position != std::string::npos)
        throw std::runtime_error ("In line " + std::to_string (current_line)
                                  + ", the header '" + line
                                  + "' contains tabs at character "
                                  + std::to_string(tab_position+1));
      if (column_line > 3)
        throw std::runtime_error ("In line " + std::to_string (current_line)
                                  + ", the table has more than 4 header lines "
                                  "starting with '|'.");
      if (column_line == 0)
        {
          ipac_column_offsets = get_bar_offsets (line);
        }
      else
        {
          check_bar_position (ipac_column_offsets, line, current_line);
        }
      /// This split creates an empty element at the beginning and
      /// end.  We keep the beginning to mark the null_bitfield_flag,
      /// and pop off the end
      boost::split (columns[column_line],line,boost::is_any_of ("|"));

      /// FIXME: I think this error can never happen, because it would
      /// have been caught by check_bar_position.
      if (columns[column_line].size()<2)
        throw std::runtime_error ("In line " + std::to_string (current_line)
                                  + ", the table is missing header information in this line: '"
                                  + line + "'");
      columns[column_line].pop_back ();
      for (auto &column: columns[column_line])
        boost::algorithm::trim (column);

      if(column_line==0)
        {
          columns[0][0]="null_bitfield_flags";
          const boost::regex name_regex("[a-zA-Z_]+[a-zA-Z0-9_]*");
          for (auto &v : columns[0])
            if (!regex_match (v, name_regex))
              throw std::runtime_error ("In line " + std::to_string (current_line)
                                        + ", the column name '" + v
                                        + "' contains an invalid character.");
        }

      if (column_line==1)
        {
          columns[1][0]="char";
          if (columns[0].size () != columns[1].size ())
            throw std::runtime_error ("Wrong number of data types in line "
                                      + std::to_string (current_line)
                                      + ". Expected "
                                      + std::to_string (columns[0].size ())
                                      + " but found "
                                      + std::to_string (columns[1].size ()));
        }

      if (column_line==2 && columns[0].size () < columns[2].size ())
        throw std::runtime_error ("Too many values for units in line  "
                                  + std::to_string (current_line)
                                  + ".  Expected at most "
                                  + std::to_string (columns[0].size ())
                                  + " but found "
                                  + std::to_string (columns[2].size ()));

      if (column_line==3 && columns[0].size () < columns[3].size ())
        throw std::runtime_error ("Too many values for null in line  "
                                  + std::to_string (current_line)
                                  + ".  Expected at most "
                                  + std::to_string (columns[0].size ())
                                  + " but found "
                                  + std::to_string (columns[3].size ()));

      ++column_line;
      first_character=ipac_file.peek ();
    }
  if (column_line < 1)
    throw std::runtime_error ("Could not find any lines starting with "
                              "'|' for the names of the columns.");
  if (column_line < 2)
    throw std::runtime_error ("Could not find any lines starting with "
                              "'|' for the data types of the columns.");
  columns[2].resize (columns[0].size ());
  columns[3].resize (columns[0].size ());
  return current_line;
}
