#include "../../../../Table.hxx"

namespace
{
tablator::Table::Type string_to_Type (const std::string &s)
{
  if (s=="boolean")
    return tablator::Table::Type::BOOLEAN;
  if (s=="short")
    return tablator::Table::Type::SHORT;
  if (s=="int")
    return tablator::Table::Type::INT;
  if (s=="long")
    return tablator::Table::Type::LONG;
  if (s=="float")
    return tablator::Table::Type::FLOAT;
  if (s=="double")
    return tablator::Table::Type::DOUBLE;
  if (s=="char")
    return tablator::Table::Type::STRING;
  // FIXME: Implement these
  if (s=="bit" || s=="byte" || s=="unicodeChar" || s=="floatComplex"
      || s=="doubleComplex")
    throw std::runtime_error ("Unimplemented data type: " + s);
  throw std::runtime_error ("Unknown data type: " + s);
}
}

std::string tablator::Table::read_field
(const boost::property_tree::ptree &field)
{
  auto child = field.begin ();
  auto end = field.end ();

  std::string name;
  Field_Properties field_properties({});
  if (child != end && child->first == "<xmlattr>")
    {
      for (auto &attribute: child->second)
        {
          if (attribute.first == "name")
            {
              name=attribute.second.get_value<std::string> ();
            }
          else if (attribute.first == "datatype")
            {
              types.push_back (string_to_Type
                               (attribute.second.get_value<std::string> ()));
            }
          // FIXME: We do not handle arrays correctly
          else if (attribute.first != "arraysize")
            {
              field_properties.attributes.insert
                (std::make_pair (attribute.first,
                                 attribute.second.get_value<std::string> ()));
            }
        }
      ++child;
    }
  
  if (child != end && child->first == "DESCRIPTION")
    {
      field_properties.description=
        child->second.get_value<std::string> ();
      ++child;
    }
  if (child != end && child->first == "VALUES")
    {
      // FIXME: read_values(*(child->second));
      ++child;
    }
  if (child != end && child->first == "LINK")
    {
      for (auto &link_child: child->second)
        {
          if (link_child.first == "<xmlattr>")
            {
              for (auto &attribute: link_child.second)
                {
                  field_properties.links.emplace_back
                    (attribute.first,
                     attribute.second.get_value<std::string> ());
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
  fields_properties.emplace_back(field_properties);
  return name;
}
