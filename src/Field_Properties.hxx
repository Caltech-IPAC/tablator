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

    inline const std::string &get_description() const { return description; }

    inline const std::map<std::string, std::string> &get_attributes() const {
        return attributes;
    }
    inline std::map<std::string, std::string> &get_attributes() { return attributes; }

    inline const Values &get_values() const { return values; }
    inline Values &get_values() { return values; }

    inline const std::vector<std::pair<std::string, std::string> > &get_links() const {
        return links;
    }
    inline std::vector<std::pair<std::string, std::string> > &get_links() {
        return links;
    }

    inline void set_description(const std::string &desc) { description.assign(desc); }
    inline void set_attributes(const std::map<std::string, std::string> &attrs) {
        attributes = attrs;
    }
    inline void set_values(const Values &vals) { values = vals; }
    inline void set_links(const std::vector<std::pair<std::string, std::string> > &ls) {
        links = ls;
    }
};
}  // namespace tablator
