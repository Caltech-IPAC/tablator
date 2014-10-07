#pragma once

namespace TAP
{
class Min_Max
{
public:
  std::string value;
  bool inclusive;

  Min_Max () : inclusive (true) {}

  bool empty () const { return value.empty (); }
};
}
