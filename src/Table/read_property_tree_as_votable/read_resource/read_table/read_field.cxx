#include "../../../../Table.hxx"
#include "VOTable_Field.hxx"

#include <boost/lexical_cast.hpp>

namespace
{
tablator::Data_Type string_to_Type (const std::string &s)
{
  if (s == "boolean")
    return tablator::Data_Type::INT8_LE;
  if (s == "unsignedByte")
    return tablator::Data_Type::UINT8_LE;
  if (s == "short")
    return tablator::Data_Type::INT16_LE;
  if (s == "ushort")
    return tablator::Data_Type::UINT16_LE;
  if (s == "int")
    return tablator::Data_Type::INT32_LE;
  if (s == "uint")
    return tablator::Data_Type::UINT32_LE;
  if (s == "long")
    return tablator::Data_Type::INT64_LE;
  if (s == "ulong")
    return tablator::Data_Type::UINT64_LE;
  if (s == "float")
    return tablator::Data_Type::FLOAT32_LE;
  if (s == "double")
    return tablator::Data_Type::FLOAT64_LE;
  if (s == "char")
    return tablator::Data_Type::CHAR;
  // FIXME: Implement these
  if (s == "bit" || s == "unicodeChar" || s == "floatComplex"
      || s == "doubleComplex")
    throw std::runtime_error ("Unimplemented data type: " + s);
  throw std::runtime_error ("Unknown data type: " + s);
}
}

tablator::VOTable_Field
tablator::Table::read_field (const boost::property_tree::ptree &field)
{
  auto child = field.begin ();
  auto end = field.end ();

  VOTable_Field result;
  if (child != end && child->first == "<xmlattr>")
    {
      for (auto &attribute : child->second)
        {
          if (attribute.first == "name")
            {
              result.name = attribute.second.get_value<std::string>();
            }
          else if (attribute.first == "datatype")
            {
              result.type
                  = string_to_Type (attribute.second.get_value<std::string>());
            }
          // FIXME: We do not handle arrays correctly
          else if (attribute.first == "arraysize")
            {
              std::string array_size = attribute.second.get_value<std::string>();
              if (array_size == "*")
                result.array_size = std::numeric_limits<size_t>::max();
              else
                result.array_size = boost::lexical_cast<size_t>(array_size);
            }
          else
            {
              result.properties.attributes.insert (std::make_pair (
                  attribute.first, attribute.second.get_value<std::string>()));
            }
        }
      ++child;
    }

  if (child != end && child->first == "DESCRIPTION")
    {
      result.properties.description = child->second.get_value<std::string>();
      ++child;
    }
  if (child != end && child->first == "VALUES")
    {
      // FIXME: read_values(*(child->second));
      ++child;
    }
  if (child != end && child->first == "LINK")
    {
      for (auto &link_child : child->second)
        {
          if (link_child.first == "<xmlattr>")
            {
              for (auto &attribute : link_child.second)
                {
                  result.properties.links.emplace_back (
                      attribute.first,
                      attribute.second.get_value<std::string>());
                }
            }
          else
            {
              throw std::runtime_error ("Expected only attributes inside "
                                        "LINK elements, but found: "
                                        + link_child.first);
            }
        }
      ++child;
    }
  return result;
}
