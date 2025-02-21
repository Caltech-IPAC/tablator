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
    Property() {}
    Property(const std::string &Value) : value_(Value) {}
    Property(const ATTRIBUTES &Attributes) : attributes_(Attributes) {}
    Property(const std::initializer_list<std::pair<const std::string, std::string>>
                     &Attributes)
            : attributes_(Attributes) {}
    Property(const std::string &Value, const ATTRIBUTES &Attributes)
            : attributes_(Attributes), value_(Value) {}

    // Called internally, directly or otherwise, only by flatten_properties().
    std::vector<STRING_PAIR> flatten(const std::string &key) const {
        std::vector<STRING_PAIR> result;
        result.push_back(std::make_pair(key, value_));
        for (auto &a : attributes_)
            result.push_back(
                    std::make_pair(key + "." + XMLATTR_DOT + a.first, a.second));
        return result;
    }

    const ATTRIBUTES &get_attributes() const { return attributes_; }
    ATTRIBUTES &get_attributes() { return attributes_; }

    const std::string &get_value() const { return value_; }
    std::string &get_value() { return value_; }

    void set_attributes(const ATTRIBUTES &attrs) { attributes_ = attrs; }
    void add_attributes(const ATTRIBUTES &attrs) {
        attributes_.insert(attrs.begin(), attrs.end());
    }

    void add_attribute(const STRING_PAIR &attr_pair) { attributes_.emplace(attr_pair); }

    void add_attribute(const std::string &name, const std::string &value) {
        add_attribute(std::make_pair(name, value));
    }

    void set_value(const std::string &val) { value_.assign(val); }

    bool empty() const { return (attributes_.empty() && value_.empty()); }

    void clear() {
        attributes_.clear();
        value_.clear();
    }

private:
    ATTRIBUTES attributes_;
    std::string value_;
};


// JTODO Make Labeled_Property a class and have the constructor
// ensure that label_ is non-empty.  This will require adjustments
// to query_server and periodogram_api.
using Labeled_Property = std::pair<std::string, Property>;
using Labeled_Properties = std::vector<Labeled_Property>;

}  // namespace tablator
