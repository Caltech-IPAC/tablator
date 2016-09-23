#include <boost/filesystem/path.hpp>

#include "DSV_Parser.hxx"

namespace tablator
{
std::list<std::vector<std::string> > parse_dsv
(const boost::filesystem::path &path)
{
  std::list<std::vector<std::string> > result;
  DSV_Parser parser(result, path.string());
  if (result.empty ())
    throw std::runtime_error ("This CSV/TSV file is empty: " + path.string());
  return result;
}
}
