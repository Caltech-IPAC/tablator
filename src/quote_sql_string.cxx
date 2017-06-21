#include <boost/regex.hpp>

#include <string>
#include <sstream>

namespace tablator
{
std::string quote_sql_string (const std::string &input, const char &quote)
{
  const boost::regex simple_characters ("^[a-zA-Z0-9_]+$");
  if (boost::regex_match (input, simple_characters))
    {
      return input;
    }
  
  std::stringstream stream;
  stream << quote;
  for (auto &c : input)
    {
      stream << c;
      if (c == quote)
        {
          stream << quote;
        }
    }
  stream << quote;
  return stream.str ();
}
}
