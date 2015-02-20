#include <boost/property_tree/ptree.hpp>
#include "../../Table.hxx"

namespace
{
void Min_Max_to_xml (boost::property_tree::ptree &tree,
                     const std::string &min_max, const Tablator::Min_Max &m)
{
  if (!min_max.empty ())
    {
      boost::property_tree::ptree &min_max_tree = tree.add (min_max, "");
      min_max_tree.add ("<xmlattr>.value", m.value);
      min_max_tree.add ("<xmlattr>.inclusive", m.inclusive ? "yes" : "no");
    }
}

void Option_to_xml (boost::property_tree::ptree &tree,
                    const Tablator::Option &option)
{
  if (!option.empty ())
    {
      boost::property_tree::ptree &option_tree = tree.add ("OPTION", "");
      option_tree.add ("<xmlattr>.name", option.name);
      option_tree.add ("<xmlattr>.value", option.value);
      for (auto &o : option.options)
        Option_to_xml (option_tree, o);
    }
}

std::string Type_to_string (const Tablator::Table::Type &type)
{
  std::string result;
  switch (type)
    {
    case Tablator::Table::Type::BOOLEAN:
      result = "boolean";
      break;
    case Tablator::Table::Type::SHORT:
      result = "short";
      break;
    case Tablator::Table::Type::INT:
      result = "int";
      break;
    case Tablator::Table::Type::LONG:
      result = "long";
      break;
    case Tablator::Table::Type::FLOAT:
      result = "float";
      break;
    case Tablator::Table::Type::DOUBLE:
      result = "double";
      break;
    case Tablator::Table::Type::STRING:
      result = "char";
      break;
    default:
      throw std::runtime_error (
          "Unexpected data type in Field_Properties_to_xml: "
          + std::to_string (static_cast<int>(type)));
    }
  return result;
}
}

namespace Tablator
{
void Field_Properties_to_xml (boost::property_tree::ptree &tree,
                              const std::string &name,
                              const Tablator::Table::Type &type,
                              const Field_Properties &field_property)
{
  boost::property_tree::ptree &field = tree.add ("FIELD", "");
  field.add ("<xmlattr>.name", name);
  std::string datatype=Type_to_string (type);
  field.add ("<xmlattr>.datatype", datatype);
  if (datatype=="char")
    field.add ("<xmlattr>.arraysize", "*");

  for (auto &a : field_property.attributes)
    {
      /// Empty attributes cause field.add to crash :(, so make sure
      /// that does not happen.

      // FIXME: This error is thrown a bit too late to be useful.

      if (a.first.empty ())
        throw std::runtime_error ("Empty attribute in field " + name
                                  + " which has type "
                                  + Type_to_string (type));
      field.add ("<xmlattr>." + a.first, a.second);
    }

  for (auto &d : field_property.descriptions)
    {
      boost::property_tree::ptree &description
          = field.add ("DESCRIPTION", d.value);
      for (auto &a : field_property.attributes)
        description.add ("<xmlattr>." + a.first, a.second);
    }

  auto &v (field_property.values);
  if (!v.empty ())
    {
      boost::property_tree::ptree &values = field.add ("VALUES", "");
      if (!v.ID.empty ())
        values.add ("<xmlattr>.ID", v.ID);
      if (!v.null.empty ())
        values.add ("<xmlattr>.null", v.null);
      if (!v.ref.empty ())
        values.add ("<xmlattr>.ref", v.ref);

      Min_Max_to_xml (values, "MIN", v.min);
      Min_Max_to_xml (values, "MAX", v.max);

      for (auto &o : v.options)
        Option_to_xml (values, o);
    }

  for (auto &l : field_property.links)
    {
      boost::property_tree::ptree &link = field.add ("LINK", "");
      for (auto &a : l)
        link.add ("<xmlattr>." + a.first, a.second);
    }
}
}
