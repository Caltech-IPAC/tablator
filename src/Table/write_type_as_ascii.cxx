#include <iostream>
#include <iomanip>

#include "../to_string.hxx"

namespace tablator
{
void write_type_as_ascii (std::ostream &os, const H5::PredType &type,
                          const char* data, const size_t &size,
                          const int &output_precision)
{   
  // FIXME: This feels slow.  Maybe move the comparison out of the
  // main loop?  Maybe it does not matter, because it has to do lots
  // of comparisons to convert to ascii anyway.
  if (type==H5::PredType::STD_I8LE)
    {
      os << static_cast<int>(*data);
    }
  else if (type==H5::PredType::STD_U8LE)
    {
      os << "0x" << std::hex
         << static_cast<const uint16_t>(static_cast<const uint8_t>(*data))
         << std::dec;
    }
  else if (type==H5::PredType::STD_I16LE)
    {
      os << *reinterpret_cast<const int16_t *>(data);
    }
  else if (type==H5::PredType::STD_U16LE)
    {
      os << *reinterpret_cast<const uint16_t *>(data);
    }
  else if (type==H5::PredType::STD_I32LE)
    {
      os << *reinterpret_cast<const int32_t *>(data);
    }
  else if (type==H5::PredType::STD_U32LE)
    {
      os << *reinterpret_cast<const uint32_t *>(data);
    }
  else if (type==H5::PredType::STD_I64LE)
    {
      os << *reinterpret_cast<const int64_t *>(data);
    }
  else if (type==H5::PredType::STD_U64LE)
    {
      os << *reinterpret_cast<const uint64_t *>(data);
    }
  else if (type==H5::PredType::IEEE_F32LE)
    {
      os << std::setprecision (output_precision)
         << *reinterpret_cast<const float *>(data);
    }
  else if (type==H5::PredType::IEEE_F64LE)
    {
      os << std::setprecision (output_precision)
         << *reinterpret_cast<const double *>(data);
    }
  else if (type==H5::PredType::C_S1)
    {
      /// The characters in the type can be shorter than the
      /// number of allowed bytes.  So add a .c_str() that
      /// will terminate the string at the first null.
      os << std::string (data,size).c_str ();
    }
  else
    {
      throw std::runtime_error ("Unknown data type when writing data: "
                                + to_string (type));
         
    }
}
}
