#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim_all.hpp>

#include "../../../../../../Data_Type.hxx"

namespace tablator
{
size_t count_elements (const std::string &entry, const Data_Type &type)
{
  if (type == Data_Type::CHAR)
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
