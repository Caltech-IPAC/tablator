#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace tablator {
class Option {
public:
    std::string name, value;
    std::vector<Option> options;

    Option(const std::string &Name, const std::string &Value)
            : name(Name), value(Value) {}
    Option(const std::initializer_list<std::string> &list) {
        if (list.size() != 2) {
            throw std::runtime_error(
                    "Wrong number of elements passed to "
                    "Option(initializer_list).  Expected 2 "
                    "but got: " +
                    std::to_string(list.size()));
        }
        name = *list.begin();
        value = *std::next(list.begin());
    }
    Option() = default;
    bool empty() const { return name.empty() && value.empty() && options.empty(); }
};
}  // namespace tablator
