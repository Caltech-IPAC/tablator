#pragma once

#include <iostream>


namespace tablator {

namespace Decimal_String_Trimmer {

// This function checks for a run of 9s or of 0s to the right of the
// decimal point in the decimal representation of doub_value. If it
// finds such a run, it rounds up or down accordingly and truncates
// the string.
const std::string get_decimal_string(double doub_value);


// This function returns the length of the string returned by get_decimal_string().
size_t get_decimal_string_length(double doub_value);

}  // namespace Decimal_String_Trimmer

}  // namespace tablator
