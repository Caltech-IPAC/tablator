#pragma once

#include <iomanip>
#include <iostream>

#include "data_size.hxx"

namespace tablator {

// This struct stores column-level data relevant to formatting
// floating-point values.  It is provided as an argument to
// Ascii_Writer functions.

struct Format_Packet {
    static const char DEFAULT_FLAG = '\0';

    //=============================================

    Format_Packet(const std::string &format_str, Data_Type data_type)
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
        auto start = format_str.find_first_not_of(" \t");
        auto end = format_str.find_last_not_of(" \t");
        if (start == std::string::npos) {
            return;
        }
        std::string trimmed_str = format_str.substr(start, end - start + 1);

        char flag = std::tolower(trimmed_str.back());
        // The only flag values used by Ascii_Writer to format
        // floating-point values are 'e', 'f', and 'g'.
        if (flag != 'e' && flag != 'f' && flag != 'g') {
            // Stick with default values.
            return;
        }
        flag_ = flag;

        // Find the dot separating field width from decimal places.
        auto dot = trimmed_str.find('.');
        if (dot == std::string::npos) {
            return;
        }
        try {
            precision_ = static_cast<uint>(std::stoi(
                    trimmed_str.substr(dot + 1, trimmed_str.size() - dot - 2)));
        } catch (const std::exception &) {
            // Stick with default value.
            return;
        }
    }

    //=============================================

    Format_Packet() : Format_Packet("", Data_Type::CHAR) {}

    //=============================================

    void apply_to_stream(std::ostream &os) const {
        switch (flag_) {
            case 'f':
                os << std::fixed << std::setprecision(precision_);
                break;
            case 'e':
                os << std::scientific << std::setprecision(precision_);
                break;
            default:
                os << std::defaultfloat << std::setprecision(precision_);
                break;
        }
    }

    //=============================================

  template <typename T, typename = std::enable_if_t<std::is_floating_point_v<T>>>
    std::string get_formatted_value(T value) const {
        std::ostringstream oss;
        apply_to_stream(oss);
        oss << value;
        return oss.str();
    }

    //=============================================

    size_t get_max_strlen() const {
        // https://stackoverflow.com/questions/2151302/counting-digits-in-a-float
        static size_t MAX_FLOAT32_STRLEN = 15;
        static size_t MAX_FLOAT64_STRLEN = 24;

        static size_t MAX_FLOAT32_EXPONENT_STRLEN = 4;
        static size_t MAX_FLOAT64_EXPONENT_STRLEN = 5;

        size_t max_strlen;
        if (data_type_ == Data_Type::FLOAT32_LE) {
            max_strlen = MAX_FLOAT32_STRLEN;
            if (flag_ == 'e') {
                max_strlen += MAX_FLOAT32_EXPONENT_STRLEN;
            }
        } else {
            max_strlen = MAX_FLOAT64_STRLEN;
            if (flag_ == 'e') {
                max_strlen += MAX_FLOAT64_EXPONENT_STRLEN;
            }
        }
        return max_strlen;
    }

    //=============================================

    Data_Type data_type_;
    uint precision_;
    char flag_;
};

}  // namespace tablator
