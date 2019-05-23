#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

#include "data_size.hxx"

namespace tablator {
void write_type_as_ascii(std::ostream &os, const Data_Type &type,
                         const size_t &array_size, const uint8_t *data,
                         const char &separator, const Data_Type &alt_datatype) {
    if (type != Data_Type::CHAR && array_size != 1) {
        for (size_t n = 0; n < array_size; ++n) {
            write_type_as_ascii(os, type, 1, data + n * data_size(type), separator,
                                alt_datatype);
            if (n != array_size - 1) os << separator;
        }
    } else {
        switch (type) {
            case Data_Type::INT8_LE:
                os << static_cast<int>(*data);
                break;
            case Data_Type::UINT8_LE: {
                std::stringstream ss;
                ss << "0x"
                   << std::hex
                   /// Extra cast here because we want to output a number, not a
                   /// character
                   << static_cast<const uint16_t>(static_cast<const uint8_t>(*data))
                   << std::dec;
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
                if (alt_datatype == Data_Type::CHAR) {
                    os << std::to_string(*reinterpret_cast<const uint64_t *>(data));
                } else if (alt_datatype == Data_Type::INT64_LE) {
                    os << *reinterpret_cast<const int64_t *>(data);
                } else {
                    os << *reinterpret_cast<const uint64_t *>(data);
                }
            } break;
            case Data_Type::FLOAT32_LE:
                os << std::setprecision(std::numeric_limits<float>::max_digits10)
                   << *reinterpret_cast<const float *>(data);
                break;
            case Data_Type::FLOAT64_LE:
                os << std::setprecision(std::numeric_limits<double>::max_digits10)
                   << *reinterpret_cast<const double *>(data);
                break;
            case Data_Type::CHAR:
                /// The characters in the type can be shorter than the
                /// number of allowed bytes.  So add a .c_str() that
                /// will terminate the string at the first null.
                os << std::string(reinterpret_cast<const char *>(data), array_size)
                                .c_str();
                break;
        }
    }
}
}  // namespace tablator
