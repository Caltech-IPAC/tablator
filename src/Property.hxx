#pragma once

#include <map>
#include <string>
#include <vector>

/// A simple class to hold properties.  It gets a little complicated
/// because XML can have attributes.
namespace tablator {
class Property {
public:
    std::map<std::string, std::string> attributes;
    std::string value;

    Property(const std::string &Value) : value(Value) {}

    std::vector<std::pair<std::string, std::string> > flatten(
            const std::string &key) const {
        std::vector<std::pair<std::string, std::string> > result;
        result.push_back(std::make_pair(key, value));
        for (auto &a : attributes)
            result.push_back(std::make_pair(key + ".<xmlattr>." + a.first, a.second));
        return result;
    }

    inline const std::map<std::string, std::string> &get_attributes() const { return attributes; }
    inline std::map<std::string, std::string> &get_attributes() { return attributes; }

    inline const std::string &get_value() const { return value; }
    inline std::string &get_value() { return value; }

    inline void set_attributes(const std::map<std::string, std::string> &attrs) {
        attributes = attrs;
    }
    inline void set_value(const std::string &val) { value.assign(val); }

    inline void add_attribute(const std::pair<std::string, std::string> &attr_pair) {
        attributes.insert(attr_pair);
    }

    inline void add_attribute(const std::string &name, const std::string &value) {
        add_attribute(std::make_pair(name, value));
    }

    inline bool empty() const {
        return (attributes.empty() && value.empty());
    }

};
}  // namespace tablator
