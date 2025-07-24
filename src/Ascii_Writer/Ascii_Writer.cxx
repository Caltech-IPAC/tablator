#include <cassert>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../Ascii_Writer.hxx"
#include "../Decimal_String_Trimmer.hxx"
#include "../data_size.hxx"

namespace tablator {
// Declare static constexpr class members.
constexpr const char Ascii_Writer::DEFAULT_SEPARATOR;
constexpr const char Ascii_Writer::IPAC_COLUMN_SEPARATOR;


//======================================================================
// TODO This function doesn't check for nulls, and calling functions
// check only is_null(), which is column-level (not
// column/array-element-level).
// =====================================================================

// The usual way to write a column value in ascii.  If the value is an
// array, individual elements are delimited by the single char
// specified in the <separator> argument.
void Ascii_Writer::write_type_as_ascii(std::ostream &os, const Data_Type &type,
                                       const size_t &array_size, const char *data,
                                       const char &separator,
                                       const Command_Line_Options &options) {
  // std::cout << "AW::write_type_as_ascii(), enter" << std::endl;

    if (type != Data_Type::CHAR && array_size != 1) {
        for (size_t n = 0; n < array_size; ++n) {
            write_array_unit_as_ascii(os, type, 1, data + n * data_size(type), options);

            if (n != array_size - 1) {
                os << separator;
            }
        }
    } else {
        write_array_unit_as_ascii(os, type, array_size, data, options);
    }
}

//======================================================================

// Called by write_single_ipac_record() as it expands a column of array
// type to multiple columns.
void Ascii_Writer::write_type_as_ascii_expand_array(
        std::ostream &os, const Data_Type &type, const size_t &array_size,
        const char *data, size_t col_width, const Command_Line_Options &options) {
    if (type != Data_Type::CHAR && array_size != 1) {
        for (size_t n = 0; n < array_size; ++n) {
            os << std::setw(col_width);
            write_array_unit_as_ascii(os, type, 1, data + n * data_size(type), options);

            if (n != array_size - 1) {
                os << IPAC_COLUMN_SEPARATOR;
            }
        }
    } else {
        write_array_unit_as_ascii(os, type, array_size, data, options);
    }
}

//=======================================================================

void Ascii_Writer::write_array_unit_as_ascii(std::ostream &os, const Data_Type &type,
                                             const size_t &array_size,
                                             const char *data,
                                             const Command_Line_Options &options) {
    if (type != Data_Type::CHAR && array_size != 1) {
        throw std::runtime_error(
                "write_array_unit_as_ascii() requires array_size == 1 if type is "
                "not CHAR");
    }
    switch (type) {
        case Data_Type::INT8_LE:
		  // std::cout << "AW: INT8_LE" << std::endl;
            os << static_cast<int>(*data);
            break;
        case Data_Type::UINT8_LE: {
		  // std::cout << "AW: UINT8_LE" << std::endl;
            std::stringstream ss;
			//            ss << "0x" << std::hex << static_cast<uint16_t>(*data) << std::dec;  // JTODO
            ss << "0x" << std::hex << (static_cast<uint16_t>(*data) & 0xFF) << std::dec;
			// std::cout << "ss.str(): " << ss.str() << std::endl;
            os << ss.str();
        } break;
        case Data_Type::INT16_LE:
		  // std::cout << "AW: INT16_LE" << std::endl;
            os << *reinterpret_cast<const int16_t *>(data);
            break;
        case Data_Type::UINT16_LE:
		  // std::cout << "AW: UINT16_LE" << std::endl;
            os << *reinterpret_cast<const uint16_t *>(data);
            break;
        case Data_Type::INT32_LE:
		  // std::cout << "AW: INT32_LE" << std::endl;
            os << *reinterpret_cast<const int32_t *>(data);
            break;
        case Data_Type::UINT32_LE:
		  // std::cout << "AW: UINT32_LE" << std::endl;
            os << *reinterpret_cast<const uint32_t *>(data);
            break;
        case Data_Type::INT64_LE:
		  // std::cout << "AW: INT64_LE" << std::endl;
            os << *reinterpret_cast<const int64_t *>(data);
            break;
        case Data_Type::UINT64_LE: {
		  // std::cout << "AW: UINT64_LE" << std::endl;
            os << *reinterpret_cast<const uint64_t *>(data);
        } break;
        case Data_Type::FLOAT32_LE: {
            // JNOTE: This might yield more digits than are warranted.
            os << std::setprecision(std::numeric_limits<float>::max_digits10)
               << *reinterpret_cast<const float *>(data);
        } break;
        case Data_Type::FLOAT64_LE: {
            if (options.is_trim_decimal_runs()) {
                os << Decimal_String_Trimmer::get_decimal_string(
                        *reinterpret_cast<const double *>(data),
                        options.min_run_length_for_trim_);
            } else {
                // JNOTE: This might yield more digits than are warranted.
                os << std::setprecision(std::numeric_limits<double>::max_digits10)
                   << *reinterpret_cast<const double *>(data);
            }
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
