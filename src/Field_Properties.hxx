#pragma once

#include <H5Cpp.h>

#include "Description.hxx"
#include "Values.hxx"

namespace Tablator
{
class Field_Properties
{
public:
  std::vector<Description> descriptions;
  std::map<std::string, std::string> attributes;
  Values values;
  std::vector<std::map<std::string, std::string> > links;

  Field_Properties (const std::map<std::string, std::string> &Attributes)
      : attributes (Attributes)
  {
  }

  Field_Properties (
      const std::initializer_list<std::pair<const std::string, std::string> > &
          Attributes)
      : attributes (Attributes)
  {
  }

  Field_Properties (
      const std::string &description,
      const std::initializer_list<std::pair<const std::string, std::string> > &
          Attributes)
      : attributes (Attributes)
  {
    descriptions.push_back (description);
  }

  Field_Properties (const std::vector<Description> &Descriptions,
                    const std::map<std::string, std::string> &Attributes)
      : descriptions (Descriptions), attributes (Attributes)
  {
  }

  Field_Properties (const std::string &description,
                    const std::map<std::string, std::string> &Attributes)
      : attributes (Attributes)
  {
    descriptions.push_back (description);
  }
};
}
