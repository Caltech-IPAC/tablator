#include <boost/property_tree/ptree.hpp>

#include "../../../Common.hxx"
#include "../../../Values.hxx"

namespace {
tablator::Min_Max read_min_max(const boost::property_tree::ptree &min_max) {
    auto child = min_max.begin();
    auto end = min_max.end();
    tablator::Min_Max result;
    if (child != end && child->first == tablator::XMLATTR) {
        for (auto &attribute : child->second) {
            if (attribute.first == tablator::ATTR_VALUE) {
                result.value = attribute.second.get_value<std::string>();
            } else if (attribute.first == "inclusive") {
                result.inclusive = attribute.second.get_value<bool>();
            }
        }
        ++child;
    }
    /// Ignore extra invalid elements
    return result;
}

tablator::Option read_option(const boost::property_tree::ptree &option) {
    auto child = option.begin();
    auto end = option.end();
    tablator::Option result;
    if (child != end && child->first == tablator::XMLATTR) {
        for (auto &attribute : child->second) {
            if (attribute.first == tablator::ATTR_NAME) {
                result.name = attribute.second.get_value<std::string>();
            } else if (attribute.first == tablator::ATTR_VALUE) {
                result.value = attribute.second.get_value<std::string>();
            }
        }
        ++child;
    }
    while (child != end && child->first == tablator::OPTION) {
        read_option(child->second);
        result.options.emplace_back(read_option(child->second));
        ++child;
    }
    /// Ignore extra invalid elements
    return result;
}
}  // namespace

namespace tablator {
Values read_values(const boost::property_tree::ptree &values) {
    auto child = values.begin();
    auto end = values.end();
    Values result;
    if (child != end && child->first == XMLATTR) {
        for (auto &attribute : child->second) {
            if (attribute.first == ID) {
                result.ID = child->second.get_value<std::string>();
            } else if (attribute.first == TYPE) {
                result.type = child->second.get_value<std::string>();
            } else if (attribute.first == "null") {
                result.null = child->second.get_value<std::string>();
            } else if (attribute.first == REF) {
                result.ref = child->second.get_value<std::string>();
            }
            /// Ignore extra invalid elements
        }
        ++child;
    }
    if (child != end && child->first == MIN) {
        result.min = read_min_max(child->second);
        ++child;
    }
    if (child != end && child->first == MAX) {
        result.max = read_min_max(child->second);
        ++child;
    }
    while (child != end && child->first == OPTION) {
        result.options.emplace_back(read_option(child->second));
        ++child;
    }
    /// Ignore extra invalid elements
    return result;
}
}  // namespace tablator
