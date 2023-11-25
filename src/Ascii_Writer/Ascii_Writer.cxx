#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "../Ascii_Writer.hxx"
#include "../data_size.hxx"

namespace {

static constexpr size_t MIN_RUN_LENGTH_FOR_ADJUSTING = 5;

//==============================================
// Helper functions for get_adjusted_string_length_for_double() and
// trim_overly_precise_double_strings().
//==============================================

// The following set of functions are used to replace trailing runs of
// 0s or of 9s of length at least MIN_RUN_LENGTH_FOR_ADJUSTING from
// representations of double values. We round positive values down if
// the run is of 0s and up if the run is of 9s, and do the opposite
// for negative values.  The rightmost decimal place of the original
// representation is ignored.  For example:

//  7.23540000001  is replaced by  7.2354.
// -7.235499999997  is replaced by -7.2355.

//==============================================

// Find the position in var_string of whichever is further to the
// right, the decimal point or the first non-zero digit.
size_t find_anchor_pos(const std::string &var_string, double var_doub) {
    size_t anchor_pos = var_string.find(".");
    if (var_doub < 1.0 && var_doub > 1.0) {
        anchor_pos = var_string.find_first_of("123456789");
    }
    return anchor_pos;
}

//==============================================

bool is_candidate_for_adjustment(size_t anchor_pos, size_t coeff_str_len) {
    return (anchor_pos != std::string::npos &&
            anchor_pos + 1 + MIN_RUN_LENGTH_FOR_ADJUSTING < coeff_str_len);
}

//==============================================

// This function assumes that is_candidate_for_adjustment() has
// already returned true.

// Returns a triple:

// strlen of rounded-off value

// the position of the MIN_RUN_LENGTH-th digit in the first run of
// either consecutive 0s or consecutive 9s to the right of anchor_pos,
// if such exists

// a flag indicating whether the run, if it exists, consists of all 9s

std::tuple<size_t, size_t, bool> get_rounding_info_for_double(
        const std::string &coeff_str, size_t anchor_pos, size_t coeff_str_len) {
    assert(is_candidate_for_adjustment(anchor_pos, coeff_str_len));

    // Set defaults and adjust as necessary.
    size_t adjusted_str_len = coeff_str_len;
    bool got_run_of_9s = false;

    // Prepare to check for a run of 0s or 9s to the right of anchor_pos.
    size_t run_start_pos = anchor_pos + 1;
    char match_val = coeff_str[run_start_pos];
    bool within_run = (match_val == '0' || match_val == '9');
    size_t run_len = (within_run) ? 1 : 0;

    size_t curr_pos = run_start_pos + 1;
    for (/* */; curr_pos < coeff_str_len; ++curr_pos) {
        char curr_val = coeff_str[curr_pos];
        if (within_run && curr_val == match_val) {
            ++run_len;
        } else if (curr_val == '0' || curr_val == '9') {
            match_val = curr_val;
            run_len = 1;
            run_start_pos = curr_pos;
            within_run = true;
        } else {
            within_run = false;
            run_len = 0;
        }

        if (run_len == MIN_RUN_LENGTH_FOR_ADJUSTING) {
            got_run_of_9s = (match_val == '9');
            adjusted_str_len = run_start_pos;
            if (adjusted_str_len == anchor_pos + 1) {
                // Retain at least one digit after the decimal point.
                ++adjusted_str_len;
            }
            break;
        }
    }
    return std::make_tuple(adjusted_str_len, curr_pos, got_run_of_9s);
}

}  // namespace


namespace tablator {

// Declare static constexpr class members.
constexpr const char Ascii_Writer::DEFAULT_SEPARATOR;
constexpr const char Ascii_Writer::IPAC_COLUMN_SEPARATOR;

//==============================================

// Called by Ipac_Table_Writer::compute_max_column_width_for_double().
size_t Ascii_Writer::get_adjusted_string_length_for_double(double var_doub) {
    std::string var_string = boost::lexical_cast<std::string>(var_doub);

    // Check for scientific notation ("a.bcd...e-MN").
    size_t e_pos = var_string.find("e");
    bool got_exp = (e_pos != std::string::npos);

    // Prepare to check for runs of 0s or of 9s starting to the right
    // of both the decimal point and the first non-zero digit.
    size_t anchor_pos = find_anchor_pos(var_string, var_doub);

    size_t str_len = var_string.size();
    size_t coeff_str_len = got_exp ? e_pos : str_len;
    size_t exp_len = got_exp ? str_len - e_pos : 0;

    if (!is_candidate_for_adjustment(anchor_pos, coeff_str_len)) {
        return str_len;
    }

    // Set default values of these variables which will then capture
    // the return value of get_rounding_info_for_double().
    size_t adjusted_coeff_str_len = coeff_str_len;
    size_t run_end_pos = 0;
    bool got_run_of_9s = false;

    std::string coeff_str = got_exp ? var_string.substr(0, e_pos) : var_string;
    std::tie(adjusted_coeff_str_len, run_end_pos, got_run_of_9s) =
            get_rounding_info_for_double(coeff_str, anchor_pos, coeff_str_len);

    return adjusted_coeff_str_len + exp_len;
}

//==============================================

// Called by Ascii_Writer::write_array_unit_as_ascii().
const std::string Ascii_Writer::trim_overly_precise_double_strings(double var_doub) {
    std::string var_string = boost::lexical_cast<std::string>(var_doub);

    // Check for scientific notation ("a.bbbbe-cc").
    size_t e_pos = var_string.find("e");
    bool got_exp = (e_pos != std::string::npos);

    // Prepare to check for runs of 0s or of 9s starting to the right
    // of both the decimal point and the first non-zero digit.
    size_t anchor_pos = find_anchor_pos(var_string, var_doub);

    size_t str_len = var_string.size();
    size_t coeff_str_len = got_exp ? e_pos : str_len;

    if (!is_candidate_for_adjustment(anchor_pos, coeff_str_len)) {
        // Early exit: this function's technique doesn't apply.
        return var_string;
    }

    // These variables will be set by the response to
    // get_rounding_info_for_double().
    size_t adjusted_coeff_str_len = coeff_str_len;
    size_t run_end_pos = 0;
    bool got_run_of_9s = false;

    std::string coeff_str = got_exp ? var_string.substr(0, e_pos) : var_string;
    std::tie(adjusted_coeff_str_len, run_end_pos, got_run_of_9s) =
            get_rounding_info_for_double(coeff_str, anchor_pos, adjusted_coeff_str_len);

    if (adjusted_coeff_str_len == coeff_str_len) {
        // Another early exit: this function's technique doesn't apply.
        return var_string;
    }

    // If we get here, var_string contains a run of 0s or of 1s to the
    // right of its anchor_pos.  We now round up or down to eliminate
    // the run and everything beyond it.  If the run is of 0s, we
    // simply truncate at the decimal point preceding the run, the
    // position indicated by adjusted_str_len.  If the run is of 9s,
    // we round up if var_doub > 0 and down if var_doub < 0.

    // Modifying the string directly to reflect rounding up or down
    // would be complicated for a run of 9s if the run continues to
    // the left past the decimal point.  Instead, if var_doub > 0, we
    // add a small value to var_doub (a 1 in the decimal place at the
    // right-hand end of the run) and then convert to string format,
    // truncating at the indicated position.  If var_doub < 0, we
    // subtract that increment rather than adding.

    // Set return value for run of 0s, in which case we just truncate
    // the original string, and then adjust for run of 9s.
    std::string ret_coeff_str = coeff_str.substr(0, adjusted_coeff_str_len);
    if (got_run_of_9s) {
        double doub_to_adjust =
                (got_exp) ? boost::lexical_cast<double>(coeff_str) : var_doub;

        double epsilon = pow(0.1, run_end_pos - anchor_pos - 1);
        short sign = (var_doub < 0) ? -1 : 1;

        ret_coeff_str = (boost::lexical_cast<std::string>(
                                 sign * ((sign * doub_to_adjust) + epsilon)))
                                .substr(0, adjusted_coeff_str_len);
    }

    if (got_exp) {
        // Append the "e-MN" to ret_coeff_str.
        return ret_coeff_str + var_string.substr(e_pos, str_len - e_pos);
    }
    return ret_coeff_str;
}


/*********************************************************************
TODO This function doesn't check for nulls, and calling functions
check only is_null(), which is column-level (not column/array-element-level).
***********************************************************************/

// The usual way to write a column value in ascii.  If the value is an
// array, individual elements are delimited by the single char
// specified in the <separator> argument.
void Ascii_Writer::write_type_as_ascii(std::ostream &os, const Data_Type &type,
                                       const size_t &array_size, const uint8_t *data,
                                       const char &separator) {
    if (type != Data_Type::CHAR && array_size != 1) {
        for (size_t n = 0; n < array_size; ++n) {
            write_array_unit_as_ascii(os, type, 1, data + n * data_size(type));

            if (n != array_size - 1) {
                os << separator;
            }
        }
    } else {
        write_array_unit_as_ascii(os, type, array_size, data);
    }
}

// Called by write_single_ipac_record() as it expands a column of array
// type to multiple columns.
void Ascii_Writer::write_type_as_ascii_expand_array(std::ostream &os,
                                                    const Data_Type &type,
                                                    const size_t &array_size,
                                                    const uint8_t *data,
                                                    size_t col_width) {
    if (type != Data_Type::CHAR && array_size != 1) {
        for (size_t n = 0; n < array_size; ++n) {
            os << std::setw(col_width);
            write_array_unit_as_ascii(os, type, 1, data + n * data_size(type));

            if (n != array_size - 1) {
                os << IPAC_COLUMN_SEPARATOR;
            }
        }
    } else {
        write_array_unit_as_ascii(os, type, array_size, data);
    }
}

void Ascii_Writer::write_array_unit_as_ascii(std::ostream &os, const Data_Type &type,
                                             const size_t &array_size,
                                             const uint8_t *data) {
    if (type != Data_Type::CHAR && array_size != 1) {
        throw std::runtime_error(
                "write_array_unit_as_ascii() requires array_size == 1 if type is not "
                "CHAR");
    }
    switch (type) {
        case Data_Type::INT8_LE:
            os << static_cast<int>(*data);
            break;
        case Data_Type::UINT8_LE: {
            std::stringstream ss;
            ss << "0x" << std::hex << static_cast<uint16_t>(*data) << std::dec;
            os << ss.str();
        } break;
        case Data_Type::INT16_LE:
            os << *reinterpret_cast<const int16_t *>(data);
            break;
        case Data_Type::UINT16_LE:
            os << *reinterpret_cast<const uint16_t *>(data);
            break;
        case Data_Type::INT32_LE:
            os << *reinterpret_cast<const int32_t *>(data);
            break;
        case Data_Type::UINT32_LE:
            os << *reinterpret_cast<const uint32_t *>(data);
            break;
        case Data_Type::INT64_LE:
            os << *reinterpret_cast<const int64_t *>(data);
            break;
        case Data_Type::UINT64_LE: {
            os << *reinterpret_cast<const uint64_t *>(data);
        } break;
        case Data_Type::FLOAT32_LE: {
            // JNOTE: This might yield more digits than are warranted.
            os << std::setprecision(std::numeric_limits<float>::max_digits10)
               << *reinterpret_cast<const float *>(data);
        } break;
        case Data_Type::FLOAT64_LE: {
            os << trim_overly_precise_double_strings(
                    *reinterpret_cast<const double *>(data));
        } break;
        case Data_Type::CHAR:
            // The number of characters in the type can be less than
            // the number of allowed bytes, so add a .c_str() that
            // will terminate the string at the first null.
            os << std::string(reinterpret_cast<const char *>(data), array_size).c_str();
            break;
    }
}
}  // namespace tablator
