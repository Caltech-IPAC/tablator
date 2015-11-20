#pragma once

#include "Min_Max.hxx"
#include "Option.hxx"

namespace tablator
{
class Values
{
public:
  Min_Max min, max;
  std::string ID, null, ref;
  std::vector<Option> options;

  bool empty () const
  {
    return empty_except_null () && null.empty ();
  }
  bool empty_except_null () const
  {
    return min.empty () && max.empty () && ID.empty ()
           && ref.empty () && options.empty ();
  }
};
}
