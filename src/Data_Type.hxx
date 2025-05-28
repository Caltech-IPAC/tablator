#pragma once

#include <limits>
#include <stdexcept>

// This enum exists only because checking what type an H5::DataType
// is requires a dynamic lookup.  This ends up dominating the run
// time if used for every row.
namespace tablator {
enum class Data_Type {
    INT8_LE,
    UINT8_LE,
    INT16_LE,
    UINT16_LE,
    INT32_LE,
    UINT32_LE,
    INT64_LE,
    UINT64_LE,
    FLOAT32_LE,
    FLOAT64_LE,
    CHAR
};


// Define Data_Type-specific get_null().

template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, T>::type get_null() {
    if (std::is_same<T, char>::value) {
        return '\0';
    }
    return std::numeric_limits<T>::max();
}

template <typename T>
inline typename std::enable_if<std::is_floating_point<T>::value, T>::type get_null() {
    return std::numeric_limits<T>::quiet_NaN();
}
}  // namespace tablator
