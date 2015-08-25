#include <boost/property_tree/ptree.hpp>
#include "../../Table.hxx"

namespace
{
void Min_Max_to_xml (boost::property_tree::ptree &tree,
                     const std::string &min_max, const tablator::Min_Max &m)
{
  if (!min_max.empty ())
    {
      boost::property_tree::ptree &min_max_tree = tree.add (min_max, "");
      min_max_tree.add ("<xmlattr>.value", m.value);
      min_max_tree.add ("<xmlattr>.inclusive", m.inclusive ? "yes" : "no");
    }
}

void Option_to_xml (boost::property_tree::ptree &tree,
                    const tablator::Option &option)
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

std::string to_string (const tablator::Table::Type &type)
{
  std::string result;
  switch (type)
    {
    case tablator::Table::Type::BOOLEAN:
      result = "boolean";
      break;
    case tablator::Table::Type::SHORT:
      result = "short";
      break;
    case tablator::Table::Type::INT:
      result = "int";
      break;
    case tablator::Table::Type::LONG:
      result = "long";
      break;
    case tablator::Table::Type::FLOAT:
      result = "float";
      break;
    case tablator::Table::Type::DOUBLE:
      result = "double";
      break;
    case tablator::Table::Type::STRING:
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

namespace tablator
{
void Field_Properties_to_xml (boost::property_tree::ptree &tree,
                              const std::string &name,
                              const tablator::Table::Type &type,
                              const Field_Properties &field_property)
{
  boost::property_tree::ptree &field = tree.add ("FIELD", "");
  field.add ("<xmlattr>.name", name);
  std::string datatype=to_string (type);
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
                                  + to_string (type));
      field.add ("<xmlattr>." + a.first, a.second);
    }

  if (!field_property.description.empty ())
    field.add ("DESCRIPTION", field_property.description);

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

  if (!field_property.links.empty ())
    {
      boost::property_tree::ptree &link = field.add ("LINK", "");
      for (auto &l : field_property.links)
        link.add ("<xmlattr>." + l.first, l.second);
    }
}
}
