#pragma once

#include "Values.hxx"

#include <map>

namespace tablator {
class Field_Properties {
public:
    std::string description;
    std::map<std::string, std::string> attributes;
    Values values;
    std::vector<std::pair<std::string, std::string> > links;

    Field_Properties() = default;

    Field_Properties(const std::map<std::string, std::string> &Attributes)
            : attributes(Attributes) {}

    Field_Properties(
            const std::initializer_list<std::pair<const std::string, std::string> >
                    &Attributes)
            : attributes(Attributes) {}

    Field_Properties(const std::string &Description) : description(Description) {}

    Field_Properties(
            const std::string &Description,
            const std::initializer_list<std::pair<const std::string, std::string> >
                    &Attributes)
            : description(Description), attributes(Attributes) {}

    Field_Properties(const std::string &Description,
                     const std::map<std::string, std::string> &Attributes)
            : description(Description), attributes(Attributes) {}

    Field_Properties(const std::string &Description,
                     const std::map<std::string, std::string> &Attributes,
                     const Values &v,
                     const std::vector<std::pair<std::string, std::string> > &Links)
            : description(Description),
              attributes(Attributes),
              values(v),
              links(Links) {}
};
}  // namespace tablator
