#include "../../../../Table.hxx"
#include "VOTable_Field.hxx"

namespace
{
H5::PredType string_to_Type (const std::string &s)
{
  if (s == "boolean")
    return H5::PredType::STD_I8LE;
  if (s == "unsignedByte")
    return H5::PredType::STD_U8LE;
  if (s == "short")
    return H5::PredType::STD_I16LE;
  if (s == "ushort")
    return H5::PredType::STD_U16LE;
  if (s == "int")
    return H5::PredType::STD_I32LE;
  if (s == "uint")
    return H5::PredType::STD_U32LE;
  if (s == "long")
    return H5::PredType::STD_I64LE;
  if (s == "ulong")
    return H5::PredType::STD_U64LE;
  if (s == "float")
    return H5::PredType::IEEE_F32LE;
  if (s == "double")
    return H5::PredType::IEEE_F64LE;
  if (s == "char")
    return H5::PredType::C_S1;
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
              result.predtype
                  = string_to_Type (attribute.second.get_value<std::string>());
            }
          // FIXME: We do not handle arrays correctly
          else if (attribute.first == "arraysize")
            {
              result.is_array = true;
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
