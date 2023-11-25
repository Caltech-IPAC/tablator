#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include <boost/lexical_cast.hpp>

#include "../Ascii_Writer.hxx"
#include "../data_size.hxx"

namespace tablator {

// Initialize static class members.
constexpr const char Ascii_Writer::DEFAULT_SEPARATOR;
constexpr const char Ascii_Writer::IPAC_COLUMN_SEPARATOR;

static constexpr size_t MIN_RUN_LENGTH_FOR_ADJUSTING = 5;

//==============================================
// Helper functions
//==============================================


//==============================================

// The following set of functions is used to replace trailing runs of
// 0s or of 9s of length at least MIN_RUN_LENGTH_FOR_ADJUSTING from
// representations of double values. We round positive values down if
// the run is of 0s and up if the run is of 9s, and do the opposite
// for negative values.  The rightmost decimal place of the original
// representation is ignored.  For example:

//  7.23540000001  is replaced by  7.2354.
// -7.235499999997  is replaced by -7.2355.

//==============================================

bool is_candidate_for_truncation(size_t point_pos, size_t str_len) {
    return (point_pos != std::string::npos &&
            point_pos < str_len - MIN_RUN_LENGTH_FOR_ADJUSTING - 1);
}

//==============================================

// This private member function assumes that
// is_candidate_for_truncation() has already returned true.
std::pair<size_t, bool> Ascii_Writer::get_adjusted_string_length_for_double(
        const std::string &string_var, size_t point_pos) {
    size_t str_len = string_var.size();
    assert(is_candidate_for_truncation(point_pos, str_len));

    // To be adjusted in due course.
    size_t adjusted_str_len = str_len;
    bool got_run_of_9s = false;

    // Check for a run of 0s or 9s preceding the rightmost digit.
    char match_val = string_var[str_len - 2];
    size_t run_len = 1;

	// We don't require the right most digit to match, but include it
	// in the run if it does.
	if (string_var[str_len - 1] == match_val) {
	  run_len = 2;
	}

    if (match_val == '0' || match_val == '9') {
        size_t pre_run_start_pos = str_len - 3;
        for (/* */; pre_run_start_pos > point_pos + 1; --pre_run_start_pos) {
            if (string_var[pre_run_start_pos] == match_val) {
                ++run_len;
            } else {
                break;
            }
        }
        if (run_len >= MIN_RUN_LENGTH_FOR_ADJUSTING) {
            got_run_of_9s = (match_val == '9');
            adjusted_str_len = pre_run_start_pos + 1;
        }
    }
    return std::make_pair(adjusted_str_len, got_run_of_9s);
}

//==============================================

size_t Ascii_Writer::get_adjusted_string_length_for_double(double doub_var) {
    std::string string_var = boost::lexical_cast<std::string>(doub_var);
    size_t str_len = string_var.size();

    size_t point_pos = string_var.find(".");
    if (!is_candidate_for_truncation(point_pos, str_len)) {
	  return str_len;
	}

    size_t adjusted_str_len = str_len;
    bool got_run_of_9s = false;
    std::tie(adjusted_str_len, got_run_of_9s) =
            get_adjusted_string_length_for_double(string_var, point_pos);
    return adjusted_str_len;
}

//==============================================

const std::string Ascii_Writer::truncate_fishy_decimals(double doub_var) {
    std::string string_var = boost::lexical_cast<std::string>(doub_var);
    size_t str_len = string_var.size();

    size_t point_pos = string_var.find(".");
    if (!is_candidate_for_truncation(point_pos, str_len)) {
        return string_var;
    }

    size_t adjusted_str_len = str_len;
    bool got_run_of_9s = false;
    std::tie(adjusted_str_len, got_run_of_9s) =
            get_adjusted_string_length_for_double(string_var, point_pos);
    if (adjusted_str_len == str_len) {
        return string_var;
    }

    // We now round up or down to eliminate the trailing run.  If the
    // run is of 0s, we simply truncate at the decimal point preceding
    // the run, the position indicated by adjusted_str_len.  If the
    // run is of 9s, we round up if doub_var > 0 and down if doub_var
    // < 0.

    // Rounding up or down manually for a run of 9s could be
    // complicated if the run continues to the left past the decimal
    // point.  Instead, if doub_var > 0, we add a small value to
    // doub_var (a 1 in the decimal place at the right-hand end of the
    // run) and then truncate at the indicated position.  If doub_var
    // < 0, we subtract that increment rather than adding.

    if (got_run_of_9s) {
        double adjust = pow(0.1, str_len - 2 - point_pos);
        short sign = (doub_var < 0) ? -1 : 1;
		string_var = boost::lexical_cast<std::string>(sign * ((sign * doub_var) + adjust));
    }
    return string_var.substr(0, adjusted_str_len);
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
            os << truncate_fishy_decimals(*reinterpret_cast<const double *>(data));
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
