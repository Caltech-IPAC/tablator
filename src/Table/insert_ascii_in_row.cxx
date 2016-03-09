#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "../Row.hxx"

namespace tablator
{
void insert_ascii_in_row (const H5::DataType &type, const size_t &column,
                          const std::string &element, const size_t &offset,
                          const size_t &offset_end, Row &row)
{
  if (type == H5::PredType::STD_I8LE)
    {
      if (element == "?" || element == " " || element[0] == '\0')
        {
          row.set_null (type, column, offset, offset_end);
        }
      else
        {
          bool result = (boost::iequals (element, "true")
                         || boost::iequals (element, "t") || element == "1");
          if (!result && !(boost::iequals (element, "false")
                           || boost::iequals (element, "f") || element == "0"))
            throw std::exception ();
          row.insert (static_cast<uint8_t>(result), offset);
        }
    }
  else if (type == H5::PredType::STD_U8LE)
    {
      /// Allow hex and octal input
      int result = std::stoi (element, nullptr, 0);
      if (result > std::numeric_limits<uint8_t>::max ()
          || result < std::numeric_limits<uint8_t>::lowest ())
        throw std::exception ();
      row.insert (static_cast<uint8_t>(result), offset);
    }
  else if (type == H5::PredType::STD_I16LE)
    {
      int result = boost::lexical_cast<int>(element);
      if (result > std::numeric_limits<int16_t>::max ()
          || result < std::numeric_limits<int16_t>::lowest ())
        throw std::exception ();
      row.insert (static_cast<int16_t>(result), offset);
    }
  else if (type == H5::PredType::STD_U16LE)
    {
      int result = boost::lexical_cast<int>(element);
      if (result > std::numeric_limits<uint16_t>::max ()
          || result < std::numeric_limits<uint16_t>::lowest ())
        throw std::exception ();
      row.insert (static_cast<uint16_t>(result), offset);
    }
  else if (type == H5::PredType::STD_I32LE)
    {
      long result = boost::lexical_cast<long>(element);
      if (result > std::numeric_limits<int32_t>::max ()
          || result < std::numeric_limits<int32_t>::lowest ())
        throw std::exception ();
      row.insert (static_cast<int32_t>(result), offset);
    }
  else if (type == H5::PredType::STD_U32LE)
    {
      long result = boost::lexical_cast<long>(element);
      if (result > std::numeric_limits<uint32_t>::max ()
          || result < std::numeric_limits<uint32_t>::lowest ())
        throw std::exception ();
      row.insert (static_cast<uint32_t>(result), offset);
    }
  else if (type == H5::PredType::STD_I64LE)
    {
      int64_t result = boost::lexical_cast<int64_t>(element);
      row.insert (result, offset);
    }
  else if (type == H5::PredType::STD_U64LE)
    {
      uint64_t result = boost::lexical_cast<uint64_t>(element);
      row.insert (result, offset);
    }
  else if (type == H5::PredType::IEEE_F32LE)
    {
      float result = boost::lexical_cast<float>(element);
      row.insert (result, offset);
    }
  else if (type == H5::PredType::IEEE_F64LE)
    {
      double result = boost::lexical_cast<double>(element);
      row.insert (result, offset);
    }
  else if (type.getClass () == H5T_STRING)
    {
      row.insert (element, offset, offset_end);
    }
  else if (type.getClass () == H5T_ARRAY)
    {
      hsize_t num_elements;
      /// We can not use ArrayType::getArrayDims because it is not const.
      H5Tget_array_dims2 (type.getId (), &num_elements);

      std::vector<std::string> elements;
      boost::split (elements, element, boost::is_any_of (" "));
      if (elements.size () != num_elements)
        throw std::runtime_error ("Expected " + std::to_string (num_elements)
                                  + " elements, but found "
                                  + std::to_string (elements.size ())
                                  + " in the cell '" + element + "'");
      auto element_offset = offset;
      auto element_size = type.getSuper ().getSize ();
      for (auto &e : elements)
        {
          insert_ascii_in_row (type.getSuper (), column, e, element_offset,
                               element_offset + element_size, row);
          element_offset += element_size;
        }
    }
  else
    {
      throw std::exception ();
    }
}
}
