#pragma once

#include <map>
#include <string>

namespace Tablator
{
class Description
{
public:
  std::string value;
  std::map<std::string, std::string> attributes;
  Description (const std::string &Value) : value (Value) {}
};
}
