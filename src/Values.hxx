#pragma once

#include "Min_Max.hxx"
#include "Option.hxx"

namespace TAP
{
class Values
{
public:
  Min_Max min, max;
  std::string ID, null, ref;
  std::vector<Option> options;

  bool empty () const
  {
    return min.empty () && max.empty () && ID.empty () && null.empty ()
           && ref.empty () && options.empty ();
  }
};
}
