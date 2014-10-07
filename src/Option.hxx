#pragma once

#include <string>
#include <vector>

namespace Tablator
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
