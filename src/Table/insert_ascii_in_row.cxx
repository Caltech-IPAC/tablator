#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>

#include "../Row.hxx"

namespace tablator
{
void insert_ascii_in_row (const H5::PredType &type,
                          const std::string &element,
                          const size_t &column,
                          const std::vector<size_t> &offsets,
                          Row &row_string)
{
  if (type==H5::PredType::STD_I8LE)
    {
      if (element=="?" || element==" " || element[0]=='\0')
        {
          row_string.set_null (column, type, offsets);
        }
      else
        {
          bool result=(boost::iequals(element, "true")
                       || boost::iequals(element, "t") || element=="1");
          if (!result && !(boost::iequals(element, "false")
                           || boost::iequals(element, "f") || element=="0"))
            throw std::exception ();
          row_string.insert (static_cast<uint8_t> (result), offsets[column]);
        }
    }
  else if (type==H5::PredType::STD_U8LE)
    {
      /// Allow hex and octal input
      int result=std::stoi (element, nullptr, 0);
      if (result > std::numeric_limits<uint8_t>::max ()
          || result < std::numeric_limits<uint8_t>::lowest ())
        throw std::exception ();
      row_string.insert (static_cast<uint8_t> (result), offsets[column]);
    }
  else if (type==H5::PredType::STD_I16LE)
    {
      int result=boost::lexical_cast<int> (element);
      if (result > std::numeric_limits<int16_t>::max ()
          || result < std::numeric_limits<int16_t>::lowest ())
        throw std::exception ();
      row_string.insert (static_cast<int16_t> (result), offsets[column]);
    }
  else if (type==H5::PredType::STD_U16LE)
    {
      int result=boost::lexical_cast<int> (element);
      if (result > std::numeric_limits<uint16_t>::max ()
          || result < std::numeric_limits<uint16_t>::lowest ())
        throw std::exception ();
      row_string.insert (static_cast<uint16_t> (result), offsets[column]);
    }
  else if (type==H5::PredType::STD_I32LE)
    {
      long result=boost::lexical_cast<long> (element);
      if (result > std::numeric_limits<int32_t>::max ()
          || result < std::numeric_limits<int32_t>::lowest ())
        throw std::exception ();
      row_string.insert (static_cast<int32_t> (result), offsets[column]);
    }
  else if (type==H5::PredType::STD_U32LE)
    {
      long result=boost::lexical_cast<long> (element);
      if (result > std::numeric_limits<uint32_t>::max ()
          || result < std::numeric_limits<uint32_t>::lowest ())
        throw std::exception ();
      row_string.insert (static_cast<uint32_t> (result), offsets[column]);
    }
  else if (type==H5::PredType::STD_I64LE)
    {
      int64_t result=boost::lexical_cast<int64_t> (element);
      row_string.insert (result, offsets[column]);
    }
  else if (type==H5::PredType::STD_U64LE)
    {
      uint64_t result=boost::lexical_cast<uint64_t> (element);
      row_string.insert (result, offsets[column]);
    }
  else if (type==H5::PredType::IEEE_F32LE)
    {
      float result=boost::lexical_cast<float> (element);
      row_string.insert (result, offsets[column]);
    }
  else if (type==H5::PredType::IEEE_F64LE)
    {
      double result=boost::lexical_cast<double> (element);
      row_string.insert (result, offsets[column]);
    }
  else if (type==H5::PredType::C_S1)
    {
      row_string.insert (element, offsets[column], offsets[column+1]);
    }
}
}
