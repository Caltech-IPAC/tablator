#pragma once

#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "../Data_Type.hxx"

namespace tablator {

// Splits text into array elements at runs of whitespace (spaces, tabs,
// newlines), ignoring leading and trailing whitespace.  Returns an empty
// vector if text is empty or all whitespace.
inline std::vector<std::string> split_array_elements(const std::string &text) {
    std::vector<std::string> elements;
    std::string trimmed = boost::trim_copy(text);
    if (trimmed.empty()) {
        return elements;
    }
    boost::split(elements, trimmed, boost::is_space(), boost::token_compress_on);
    return elements;
}

//========================================

// Parses column_null_str as a value of type T.  Returns false if it
// isn't such a value, e.g. if it is the default null string or an
// out-of-range number.
template <typename T>
bool parse_column_null_str(T &null_value, const std::string &column_null_str) {
    static_assert(std::is_integral_v<T>, "integer types only");
    const char *first = column_null_str.data();
    const char *last = first + column_null_str.size();
    auto [ptr, ec] = std::from_chars(first, last, null_value);
    return ec == std::errc() && ptr == last;
}

//========================================

// Returns true if column_null_str is a valid value of the integer type T
// other than tablator's own null value for T.
template <typename T>
bool is_non_default_null_str(const std::string &column_null_str) {
    static_assert(std::is_integral_v<T>, "integer types only");
    T null_value = 0;
    return parse_column_null_str(null_value, column_null_str) &&
           null_value != get_null<T>();
}

//========================================

// Returns true if data_type is an integer type and column_null_str is a
// valid value of data_type other than tablator's null value for that type.
inline bool is_non_default_integer_null_str(const std::string &column_null_str,
                                            Data_Type data_type) {
    switch (data_type) {
        case Data_Type::UINT8_LE:
            return is_non_default_null_str<uint8_t>(column_null_str);
        case Data_Type::INT16_LE:
            return is_non_default_null_str<int16_t>(column_null_str);
        case Data_Type::UINT16_LE:
            return is_non_default_null_str<uint16_t>(column_null_str);
        case Data_Type::INT32_LE:
            return is_non_default_null_str<int32_t>(column_null_str);
        case Data_Type::UINT32_LE:
            return is_non_default_null_str<uint32_t>(column_null_str);
        case Data_Type::INT64_LE:
            return is_non_default_null_str<int64_t>(column_null_str);
        default:
            return false;
    }
}

}  // namespace tablator
