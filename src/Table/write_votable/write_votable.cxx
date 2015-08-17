#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "../../Table.hxx"

namespace tablator
{
void Field_Properties_to_xml (boost::property_tree::ptree &tree,
                              const std::string &name,
                              const tablator::Table::Type &type,
                              const Field_Properties &field_property);
}

void tablator::Table::write_votable (std::ostream &os) const
{
  boost::property_tree::ptree tree;
  tree.add ("VOTABLE.<xmlattr>.version", "1.3");
  tree.add ("VOTABLE.<xmlattr>.xmlns:xsi",
            "http://www.w3.org/2001/XMLSchema-instance");
  tree.add ("VOTABLE.<xmlattr>.xmlns", "http://www.ivoa.net/xml/VOTable/v1.3");
  tree.add ("VOTABLE.<xmlattr>.xmlns:stc",
            "http://www.ivoa.net/xml/STC/v1.30");

  // VOTable only allows a single DESCRIPTION element, so we have to
  // cram all of the comments into a single line
  if (!comments.empty ())
    {
      std::string description;
      for (auto &c: comments)
        description+=c + '\n';
      if (!description.empty ())
        tree.add ("VOTABLE.DESCRIPTION",
                  description.substr (0,description.size ()-1));
    }
  bool overflow=false;
  for (auto &p : flatten_properties ())
    {
      if (boost::starts_with (p.first,"VOTABLE."))
        {
          tree.add (p.first, p.second);
        }
      else if (boost::starts_with (p.first, "<xmlattr>.")
               || boost::starts_with (p.first, "COOSYS.")
               || boost::starts_with (p.first, "GROUP.")
               || boost::starts_with (p.first, "PARAM.")
               || boost::starts_with (p.first, "INFO."))
        {
          tree.add ("VOTABLE." + p.first, p.second);
        }
      else if (p.first!="OVERFLOW")
        {
          auto &info = tree.add ("VOTABLE.RESOURCE.INFO", "");
          info.add ("<xmlattr>.name", p.first);
          info.add ("<xmlattr>.value", p.second);
        }
      else
        {
          overflow=true;
        }
    }

  boost::property_tree::ptree &table = tree.add ("VOTABLE.RESOURCE.TABLE", "");
  /// Skip null_bitfield_flag
  for (size_t i = 1; i < fields_properties.size (); ++i)
    Field_Properties_to_xml (table, compound_type.getMemberName (i),
                             types.at (i), fields_properties[i]);

  boost::property_tree::ptree &tabledata = table.add ("DATA.TABLEDATA", "");
  put_table_in_property_tree (tabledata);

  if (overflow)
    {
      auto &info = tree.add ("VOTABLE.RESOURCE.INFO", "");
      info.add ("<xmlattr>.name", "QUERY_STATUS");
      info.add ("<xmlattr>.value", "OVERFLOW");
    }
  
  boost::property_tree::write_xml (os, tree, boost::property_tree::xml_writer_make_settings(' ',2));
}
