#pragma once

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <system_error>
#include <type_traits>

#include "data_size.hxx"

namespace tablator {

// This struct stores column-level data relevant to formatting
// floating-point values.  It is provided as an argument to
// Ascii_Writer functions.

struct Format_Packet {
    static const char DEFAULT_FLAG = '\0';

    //=============================================

    // precision_str is the value of a column's `precision` attribute.
    // Supported values here consist of an optional flag letter ('E', 'F',
    // or 'G', case insensitive) followed by a number, e.g. "E10", "F6",
    // "G4".  A bare number "N" is interpreted as "FN".  printf
    // semantics apply: for 'E' and 'F' the number is the count of
    // digits after the decimal point; for 'G' it is the count of
    // significant digits.  A missing or malformed value leaves the
    // defaults in place.
    Format_Packet(const std::string &precision_str, Data_Type data_type)
            : data_type_(data_type), precision_(0), flag_(DEFAULT_FLAG) {
        if ((data_type != Data_Type::FLOAT32_LE) &&
            (data_type != Data_Type::FLOAT64_LE)) {
            // Stick with default values for non-floating-point types.
            return;
        }

        // Update default precision_ value.
        if (data_type == Data_Type::FLOAT32_LE) {
            precision_ = std::numeric_limits<float>::max_digits10;
        } else {
            precision_ = std::numeric_limits<double>::max_digits10;
        }

        // Strip leading/trailing whitespace.
        auto start = precision_str.find_first_not_of(" \t");
        if (start == std::string::npos) {
            return;
        }
        auto end = precision_str.find_last_not_of(" \t");
        const char *first = precision_str.data() + start;
        const char *last = precision_str.data() + end + 1;

        char flag = 'f';
        if (std::isalpha(static_cast<unsigned char>(*first))) {
            flag = static_cast<char>(std::tolower(static_cast<unsigned char>(*first)));
            // The only flag values used by Ascii_Writer to format
            // floating-point values are (lowercase) 'e', 'f', and
            // 'g'.
            if (flag != 'e' && flag != 'f' && flag != 'g') {
                // Stick with default values.
                return;
            }
            ++first;
        }
        if (first == last) {
            return;
        }

        unsigned int precision = 0;
        auto [ptr, ec] = std::from_chars(first, last, precision);
        if (ec != std::errc() || ptr != last) {
            // Stick with default values.
            return;
        }

        // Set non-default values only after the whole string has parsed.
        flag_ = flag;
        precision_ = precision;
    }

    //=============================================

    Format_Packet() : Format_Packet("", Data_Type::CHAR) {}

    //=============================================

    // Sets the stream's notation and precision for this column, then
    // outputs the value. Any width set on the stream beforehand
    // applies to the value.
    template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    void write_to_stream(std::ostream &os, T value) const {
        switch (flag_) {
            case 'f':
                os << std::fixed;
                break;
            case 'e':
                os << std::scientific;
                break;
            case 'g':
            default:
                os << std::defaultfloat;
                break;
        }
        os << std::setprecision(precision_) << value;
    }

    //=============================================

    template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    std::string get_formatted_value(T value) const {
        std::ostringstream oss;
        write_to_stream(oss, value);
        return oss.str();
    }

    //=============================================

    // Upper bound on the length of any string write_to_stream() produces
    // for this column, including a minus sign.
    size_t get_max_strlen() const {
        bool is_float32 = (data_type_ == Data_Type::FLOAT32_LE);
        // The integer part of the type's largest value: 39 digits for float,
        // 309 for double.
        size_t max_integer_digits = std::numeric_limits<double>::max_exponent10 + 1;
        if (is_float32) {
            max_integer_digits = std::numeric_limits<float>::max_exponent10 + 1;
        }
        // "e+38" for float, "e+308" for double.
        size_t exponent_strlen = is_float32 ? 4 : 5;
        size_t sign_strlen = 1;
        size_t point_strlen = (precision_ > 0) ? 1 : 0;

        // -d.ddde+xx, with precision_ digits after the point.
        size_t scientific_strlen =
                sign_strlen + 1 + point_strlen + precision_ + exponent_strlen;

        switch (flag_) {
            case 'e':
                return scientific_strlen;
            case 'f':
                // -ddd.ddd, with every digit of the integer part and
                // precision_ digits after the point.
                return sign_strlen + max_integer_digits + point_strlen + precision_;
            case 'g':
            default: {
                // std::defaultfloat prints at most this many
                // significant digits, either as -d.ddde+xx or, for
                // exponents down to -4, as -0.000ddd.
                size_t significant_digits = std::max(precision_, 1u);
                size_t general_scientific_strlen =
                        sign_strlen + significant_digits + 1 + exponent_strlen;
                size_t general_fixed_strlen = sign_strlen + 5 + significant_digits;
                return std::max(general_scientific_strlen, general_fixed_strlen);
            }
        }
    }

    //=============================================

    Data_Type data_type_;
    unsigned int precision_;
    char flag_;
};

}  // namespace tablator
