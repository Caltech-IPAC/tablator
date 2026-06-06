#pragma once

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

    Data_Type data_type_;
    uint precision_;
    char flag_;
};

}  // namespace tablator
