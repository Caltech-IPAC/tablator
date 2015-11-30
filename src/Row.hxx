#pragma once

#include <vector>
#include <algorithm>
#include <cassert>

#include <H5Cpp.h>
#include "unsafe_copy_to_row.hxx"

namespace tablator
{
class Row
{
public:
  std::vector<char> data;

  Row (const size_t &size): data(size) {}
  
  // FIXME: This clears everything, not just nulls
  void set_zero ()
  {
    std::fill (data.begin (), data.end (), 0);
  }

  void set_null (const H5::DataType &type, const size_t &column,
                 const size_t &offset, const size_t &offset_end);

  template <typename T>
  void insert (const T &element, const size_t &offset)
  {
    assert (offset + sizeof(T) <= data.size ());
    unsafe_copy_to_row (element, offset, data.data ());
  }

  template <typename T>
  void insert (const T &begin, const T &end, const size_t &offset)
  {
    assert (offset + sizeof(T)*std::distance(begin,end) < data.size ());
    std::copy (begin, end, data.data () + offset);
  }

  void insert (const std::string &element, const size_t &offset_begin,
               const size_t &offset_end)
  {
    std::string element_copy(element);
    element_copy.resize (offset_end-offset_begin,'\0');
    std::copy (element_copy.begin (), element_copy.end (),
               data.data () + offset_begin);
  }

};
}
