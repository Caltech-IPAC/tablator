#include <vector>

#include <H5Cpp.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>

namespace tablator
{
size_t count_elements (const std::string &entry, const H5::PredType &predtype)
{
  if (predtype == H5::PredType::C_S1)
    {
      return entry.size ();
    }
  else
    {
      std::vector<std::string> elements;
      std::string trimmed (boost::trim_all_copy (entry));
      boost::split (elements, trimmed, boost::is_any_of (" "));
      return elements.size ();
    }
}
}
