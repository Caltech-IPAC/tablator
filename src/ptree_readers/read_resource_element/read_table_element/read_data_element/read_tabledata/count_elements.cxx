#include "../../../../../Data_Type.hxx"
#include "../../../../../Utils/String_Utils.hxx"

namespace tablator {
size_t count_elements(const std::string &entry, const Data_Type &type) {
    if (type == Data_Type::CHAR) {
        return entry.size();
    } else {
        return split_array_elements(entry).size();
    }
}
}  // namespace tablator
