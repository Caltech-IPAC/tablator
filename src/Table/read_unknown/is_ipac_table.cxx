#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>

namespace tablator
{
bool is_ipac_table (const char &first_character)
{
  return first_character=='\\' || first_character=='|';
}

bool is_ipac_table (std::istream &input_stream)
{
  char first_character = input_stream.peek ();
  return input_stream.good () && is_ipac_table (first_character);
}
  
bool is_ipac_table (const boost::filesystem::path &path)
{
  boost::filesystem::ifstream stream (path);
  return is_ipac_table (stream);
}
}
