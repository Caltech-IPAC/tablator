#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>

namespace tablator
{
bool is_votable (const char &first_character)
{
  return first_character == '<';
}

bool is_votable (std::istream &input_stream)
{
  char first_character = input_stream.peek ();
  return input_stream.good () && is_votable (first_character);
}
  
bool is_votable (const boost::filesystem::path &path)
{
  boost::filesystem::ifstream stream (path);
  return is_votable (stream);
}
}
