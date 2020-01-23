#pragma once

#include <map>
#include <string>
#include <vector>

/// A simple class to hold properties.  It gets a little complicated
/// because XML can have attributes.
namespace tablator {
class Property {
public:
    Property(const std::string &Value) : value_(Value) {}

    std::vector<std::pair<std::string, std::string> > flatten(
            const std::string &key) const {
        std::vector<std::pair<std::string, std::string> > result;
        result.push_back(std::make_pair(key, value_));
        for (auto &a : attributes_)
            result.push_back(
                    std::make_pair(key + ".<xmlattr>." + a.first, a.second));
        return result;
    }

    inline const std::map<std::string, std::string> &get_attributes() const { return attributes_; }
    inline std::map<std::string, std::string> &get_attributes() { return attributes_; }

    inline const std::string &get_value() const { return value_; }
    inline std::string &get_value() { return value_; }

    inline void set_attributes(const std::map<std::string, std::string> &attrs) {
        attributes_ = attrs;
    }
    inline void set_value(const std::string &val) { value_.assign(val); }

    inline void add_attribute(const std::pair<std::string, std::string> &attr_pair) {
        attributes_.insert(attr_pair);
    }

    inline void add_attribute(const std::string &name, const std::string &value) {
        add_attribute(std::make_pair(name, value));
    }

    inline bool empty() const {
        return (attributes_.empty() && value_.empty());
    }

private:
    std::map<std::string, std::string> attributes_;
    std::string value_;


};
}  // namespace tablator
