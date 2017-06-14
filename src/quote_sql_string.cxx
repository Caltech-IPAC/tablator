#include <string>
#include <sstream>

namespace tablator
{
std::string quote_sql_string (const std::string &input, const char &quote)
{
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
