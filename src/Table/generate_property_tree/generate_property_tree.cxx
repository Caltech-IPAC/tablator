#include <boost/algorithm/string/predicate.hpp>

#include "../../Table.hxx"

namespace tablator
{
void Field_Properties_to_property_tree (boost::property_tree::ptree &tree,
                                        const std::string &name,
                                        const H5::DataType &type,
                                        const Field_Properties &field_property);
}

boost::property_tree::ptree tablator::Table::generate_property_tree () const
{
  boost::property_tree::ptree tree;
  std::string votable_literal ("VOTABLE");
  auto &votable=tree.add (votable_literal,"");
  votable.add ("<xmlattr>.version", "1.3");
  votable.add ("<xmlattr>.xmlns:xsi",
               "http://www.w3.org/2001/XMLSchema-instance");
  votable.add ("<xmlattr>.xmlns", "http://www.ivoa.net/xml/VOTable/v1.3");
  votable.add ("<xmlattr>.xmlns:stc",
               "http://www.ivoa.net/xml/STC/v1.30");

  // VOTable only allows a single DESCRIPTION element, so we have to
  // cram all of the comments into a single line
  if (!comments.empty ())
    {
      std::string description;
      for (auto &c: comments)
        description+=c + '\n';
      if (!description.empty ())
        votable.add ("DESCRIPTION",
                     description.substr (0,description.size ()-1));
    }
  bool overflow=false;
  const std::string resource_literal ("RESOURCE"), table_literal ("TABLE");
  auto &resource=votable.add (resource_literal,"");
  for (auto &p: properties)
    {
      if (p.first==votable_literal)
        continue;
      if (boost::starts_with (p.first,votable_literal + "."))
        {
          votable.add (p.first.substr(votable_literal.size ()+1),
                       p.second.value);
        }
      else if (p.first=="COOSYS" || p.first=="GROUP" || p.first=="PARAM"
               || p.first=="INFO")
        {
          auto &element=votable.add (p.first, p.second.value);
          for (auto &a: p.second.attributes)
            element.add ("<xmlattr>." + a.first, a.second);
        }
      else if (p.first==resource_literal)
        {
          for (auto &a: p.second.attributes)
            resource.add ("<xmlattr>." + a.first, a.second);
        }
      else if (boost::starts_with (p.first, resource_literal + "." + table_literal))
        {
          /// Skip TABLE for now.
        }
      else if (boost::starts_with (p.first, resource_literal + "."))
        {
          auto &element=resource.add (p.first.substr(resource_literal.size () +1),
                                      p.second.value);
          for (auto &a: p.second.attributes)
            element.add ("<xmlattr>." + a.first, a.second);
        }
      else if (p.first=="OVERFLOW")
        {
          overflow=true;
        }
      else
        {
          auto &info = resource.add ("INFO", "");
          info.add ("<xmlattr>.name", p.first);
          info.add ("<xmlattr>.value", p.second.value);
          for (auto &a: p.second.attributes)
            {
              auto &info_attribute = resource.add ("INFO", "");
              info_attribute.add ("<xmlattr>.name", p.first + "." + a.first);
              info_attribute.add ("<xmlattr>.value", a.second);
            }
        }
    }

  boost::property_tree::ptree &table = resource.add (table_literal, "");

  for (auto &p: properties)
    {
      if (boost::starts_with (p.first, resource_literal + "." + table_literal))
        {
          for (auto &a: p.second.attributes)
            table.add ("<xmlattr>." + a.first, a.second);
        }
    }
  /// Skip null_bitfield_flag
  for (size_t i = 1; i < fields_properties.size (); ++i)
    Field_Properties_to_property_tree (table, compound_type.getMemberName (i),
                                       compound_type.getMemberDataType (i),
                                       fields_properties[i]);

  boost::property_tree::ptree &tabledata = table.add ("DATA.TABLEDATA", "");
  put_table_in_property_tree (tabledata);

  if (overflow)
    {
      auto &info = tree.add ("VOTABLE.RESOURCE.INFO", "");
      info.add ("<xmlattr>.name", "QUERY_STATUS");
      info.add ("<xmlattr>.value", "OVERFLOW");
    }

  return tree;
}
