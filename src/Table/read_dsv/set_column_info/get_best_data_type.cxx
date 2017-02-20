#include "../../../Data_Type.hxx"

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <string>

namespace tablator
{
Data_Type get_best_data_type (const Data_Type &current_type,
                              const std::string &element)
{
  Data_Type result (current_type);
  /// This uses fallthrough to progressively promote the type when
  /// the conversion fails
  if (!element.empty())
    {
      switch (result)
        {
        case Data_Type::INT8_LE:
          if (element=="0" || element=="1" || boost::iequals(element,"true")
              || boost::iequals(element,"false")
              || boost::iequals(element,"t") || boost::iequals(element,"f"))
            break;
          result=Data_Type::UINT8_LE;
        case Data_Type::UINT8_LE:
          try
            {
              size_t number_of_chars_processed;
              int result = std::stoi (element, &number_of_chars_processed, 0);
              if (number_of_chars_processed == element.size ()
                  && !(result > std::numeric_limits<uint8_t>::max ()
                       || result < std::numeric_limits<uint8_t>::lowest ()))
                break;
            }
          catch(std::exception &)
            {}
          result=Data_Type::INT64_LE;
        case Data_Type::INT64_LE:
          try
            {
              boost::lexical_cast<int64_t>(element);
              break;
            }
          catch(const boost::bad_lexical_cast &)
            {
              result=Data_Type::UINT64_LE;
            }
        case Data_Type::UINT64_LE:
          try
            {
              boost::lexical_cast<uint64_t>(element);
              break;
            }
          catch(const boost::bad_lexical_cast &)
            {
              result=Data_Type::FLOAT64_LE;
            }
        case Data_Type::FLOAT64_LE:
          try
            {
              boost::lexical_cast<double>(element);
              break;
            }
          catch(const boost::bad_lexical_cast &)
            {
              result=Data_Type::CHAR;
            }
        case Data_Type::CHAR:
          break;
        default:
          throw std::runtime_error("INTERNAL ERROR: Invalid data type in "
                                   "update_data_type");
        }
    }
  return result;
}
}
