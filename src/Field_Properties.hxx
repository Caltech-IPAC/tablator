#pragma once

#include "Values.hxx"

#include <map>

namespace tablator {
class Field_Properties {
public:
    Field_Properties() = default;

    Field_Properties(const std::map<std::string, std::string> &Attributes)
            : attributes_(Attributes) {}

    Field_Properties(
            const std::initializer_list<std::pair<const std::string, std::string> >
                    &Attributes)
            : attributes_(Attributes) {}

    Field_Properties(const std::string &Description) : description_(Description) {}

    Field_Properties(
            const std::string &Description,
            const std::initializer_list<std::pair<const std::string, std::string> >
                    &Attributes)
            : description_(Description), attributes_(Attributes) {}

    Field_Properties(const std::string &Description,
                     const std::map<std::string, std::string> &Attributes)
            : description_(Description), attributes_(Attributes) {}

    Field_Properties(const std::string &Description,
                     const std::map<std::string, std::string> &Attributes,
                     const Values &v,
                     const std::vector<std::pair<std::string, std::string> > &Links)
            : description_(Description),
              attributes_(Attributes),
              values_(v),
              links_(Links) {}

    inline const std::string &get_description() const { return description_; }

    inline const std::map<std::string, std::string> &get_attributes() const {
        return attributes_;
    }
    inline std::map<std::string, std::string> &get_attributes() { return attributes_; }

    inline const Values &get_values() const { return values_; }
    inline Values &get_values() { return values_; }

    inline const std::vector<std::pair<std::string, std::string> > &get_links() const {
        return links_;
    }
    inline std::vector<std::pair<std::string, std::string> > &get_links() {
        return links_;
    }

    inline void set_description(const std::string &desc) { description_.assign(desc); }
    inline void set_attributes(const std::map<std::string, std::string> &attrs) {
        attributes_ = attrs;
    }
    inline void set_values(const Values &vals) { values_ = vals; }
    inline void set_links(const std::vector<std::pair<std::string, std::string> > &ls) {
        links_ = ls;
    }


    inline void add_attribute(const std::pair<std::string, std::string> &attr_pair) {
        attributes_.insert(attr_pair);
    }

    inline void add_attribute(const std::string &name, const std::string &value) {
        add_attribute(std::make_pair(name, value));
    }

    inline void add_link(const std::pair<std::string, std::string> &link) {
        links_.emplace_back(link);
    }

    inline void add_link(const std::string &role, const std::string &href) {
        add_link(std::make_pair(role, href));
    }

private:
    std::string description_;
    std::map<std::string, std::string> attributes_;
    Values values_;
    std::vector<std::pair<std::string, std::string> > links_;
};
}  // namespace tablator
