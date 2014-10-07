#pragma once

/// A simple class to hold properties.  It gets a little complicated
/// because XML can have attributes.
class Property
{
public:
  std::map<std::string, std::string> attributes;
  std::string value;

  Property (const std::string &Value) : value (Value) {}

  std::vector<std::pair<std::string, std::string> >
  flatten (const std::string &key) const
  {
    std::vector<std::pair<std::string, std::string> > result;
    result.push_back (std::make_pair (key, value));
    for (auto &a : attributes)
      result.push_back (std::make_pair (key + "." + a.first, a.second));
    return result;
  }
};
