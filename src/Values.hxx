#pragma once

#include "Min_Max.hxx"
#include "Option.hxx"

namespace tablator {
class Values {
public:
    Min_Max min, max;
    std::string ID, type, null, ref;
    std::vector<Option> options;

    Values() = default;
    Values(const Min_Max &Min, const Min_Max &Max, const std::string &id,
           const std::string &Type, const std::string &Null, const std::string &Ref,
           const std::vector<Option> &Options)
            : min(Min),
              max(Max),
              ID(id),
              type(Type),
              null(Null),
              ref(Ref),
              options(Options) {}
    bool empty() const { return empty_except_null() && null.empty(); }
    bool empty_except_null() const {
        return min.empty() && max.empty() && ID.empty() && type.empty() &&
               ref.empty() && options.empty();
    }
};
}  // namespace tablator
