#include <sstream>
#include <iostream>
#include <iomanip>

#include "data_size.hxx"

namespace tablator
{
void write_type_as_ascii (std::ostream &os, const Data_Type &type,
                          const size_t &array_size,
                          const uint8_t *data,
                          const int &output_precision)
{
  if (array_size != 1 && type != Data_Type::CHAR)
    {
      for (size_t n = 0; n < array_size; ++n)
        {
          write_type_as_ascii (os, type, 1, data + n * data_size (type),
                               output_precision);
          if (n != array_size - 1)
            os << ' ';
        }
    }
  else
    {
      switch (type)
        {
        case Data_Type::INT8_LE:
          os << static_cast<int>(*data);
          break;
        case Data_Type::UINT8_LE:
          {
            std::stringstream ss;
            ss << "0x" << std::hex
              /// Extra cast here because we want to output a number, not a
              /// character
               << static_cast<const uint16_t>(static_cast<const uint8_t>(*data))
               << std::dec;
            os << ss.str ();
          }
          break;
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
        case Data_Type::UINT64_LE:
          os << *reinterpret_cast<const uint64_t *>(data);
          break;
        case Data_Type::FLOAT32_LE:
          os << std::setprecision (output_precision)
             << *reinterpret_cast<const float *>(data);
          break;
        case Data_Type::FLOAT64_LE:
          os << std::setprecision (output_precision)
             << *reinterpret_cast<const double *>(data);
          break;
        case Data_Type::CHAR:
          /// The characters in the type can be shorter than the
          /// number of allowed bytes.  So add a .c_str() that
          /// will terminate the string at the first null.
          os << std::string (reinterpret_cast<const char*>(data),
                             array_size).c_str ();
          break;
        }
    }
}
}
