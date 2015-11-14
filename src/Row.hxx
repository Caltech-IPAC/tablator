#pragma once

#include <vector>
#include <algorithm>
#include <cassert>

#include "Type.hxx"

namespace tablator
{
class Row
{
public:
  std::vector<char> data;

  Row (const size_t &size): data(size) {}
  
  // FIXME: This clears everything, not just nulls
  void clear_nulls ()
  {
    std::fill (data.begin (), data.end (), 0);
  }

  void set_null (size_t column, const Type &type,
                 const std::vector<size_t> offsets);

  template <typename T>
  void copy_to_row (const T &element, const size_t &offset)
  {
    assert (offset + sizeof(T) <= data.size ());
    // FIXME: This might be undefined, because element+1 is not
    // guaranteed to be valid
    std::copy (reinterpret_cast<const char *>(&element),
               reinterpret_cast<const char *>(&element + 1),
               data.data () + offset);
  }

  template <typename T>
  void copy_to_row (const T &begin, const T &end, const size_t &offset)
  {
    assert (offset + sizeof(T)*std::distance(begin,end) < data.size ());
    std::copy (begin, end, data.data () + offset);
  }

  void copy_to_row (const std::string &element, const size_t &offset_begin,
                    const size_t &offset_end)
  {
    std::string element_copy(element);
    element_copy.resize (offset_end-offset_begin,'\0');
    std::copy (element_copy.begin (), element_copy.end (),
               data.data () + offset_begin);
  }

};
}
