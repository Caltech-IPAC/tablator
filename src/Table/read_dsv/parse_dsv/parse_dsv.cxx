#include <boost/filesystem/path.hpp>

#include "DSV_Parser.hxx"

namespace tablator
{
std::list<std::vector<std::string> >
parse_dsv (std::istream &input_stream, const char &delimiter)
{
  std::list<std::vector<std::string> > result;
  DSV_Parser parser (result, input_stream, delimiter);
  if (result.empty ())
    {
      throw std::runtime_error ("The CSV/TSV stream is empty");
    }
  return result;
}
}
