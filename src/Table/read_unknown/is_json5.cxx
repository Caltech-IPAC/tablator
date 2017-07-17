#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem.hpp>

namespace tablator
{
bool is_json5 (const char &first_character)
{
  return first_character=='{' || first_character=='[';
}

bool is_json5 (std::istream &input_stream)
{
  char first_character = input_stream.peek ();
  return input_stream.good () && is_json5 (first_character);
}

bool is_json5 (const boost::filesystem::path &path)
{
  boost::filesystem::ifstream stream (path);
  return is_json5 (stream);
}
}
