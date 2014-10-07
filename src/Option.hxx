#pragma once

#include <string>
#include <vector>

namespace TAP
{
class Option
{
public:
  std::string name, value;
  std::vector<Option> options;

  bool empty () const
  {
    return name.empty () && value.empty () && options.empty ();
  }
};
}
