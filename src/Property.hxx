#pragma once

#include <map>
#include <string>
#include <vector>

#include "Common.hxx"

// A simple class to hold properties.  It gets a little complicated
// because XML can have attributes.

namespace tablator {
class Property {
public:
    // JTODO Builder
    Property(const std::string &Value) : value_(Value) {}
    Property(const ATTRIBUTES &Attributes) : attributes_(Attributes) {}
    Property(const std::string &Value, const ATTRIBUTES &Attributes)
            : value_(Value), attributes_(Attributes) {}

    // Called internally, directly or otherwise, only by flatten_properties().
    std::vector<std::pair<std::string, std::string> > flatten(
            const std::string &key) const {
        std::vector<std::pair<std::string, std::string> > result;
        result.push_back(std::make_pair(key, value_));
        for (auto &a : attributes_)
            result.push_back(
                    std::make_pair(key + "." + XMLATTR_DOT + a.first, a.second));
        return result;
    }

    inline const ATTRIBUTES &get_attributes() const { return attributes_; }
    inline ATTRIBUTES &get_attributes() { return attributes_; }

    inline const std::string &get_value() const { return value_; }
    inline std::string &get_value() { return value_; }

    inline void set_attributes(const ATTRIBUTES &attrs) { attributes_ = attrs; }
    inline void set_value(const std::string &val) { value_.assign(val); }

    inline void add_attribute(const std::pair<std::string, std::string> &attr_pair) {
        attributes_.insert(attr_pair);
    }

    inline void add_attribute(const std::string &name, const std::string &value) {
        add_attribute(std::make_pair(name, value));
    }

    inline bool empty() const { return (attributes_.empty() && value_.empty()); }

private:
    std::string value_;
    ATTRIBUTES attributes_;
};
}  // namespace tablator
