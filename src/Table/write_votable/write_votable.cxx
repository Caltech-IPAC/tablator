#include <boost/property_tree/xml_parser.hpp>

#include "../../Table.hxx"

namespace TAP
{
void Field_Properties_to_xml (boost::property_tree::ptree &tree,
                              const std::string &name,
                              const TAP::Table::Type &type,
                              const Field_Properties &field_property);
}

void TAP::Table::write_votable (std::ostream &os) const
{
  boost::property_tree::ptree tree;
  tree.add ("VOTABLE.<xmlattr>.version", "1.3");
  tree.add ("VOTABLE.<xmlattr>.xmlns:xsi",
            "http://www.w3.org/2001/XMLSchema-instance");
  tree.add ("VOTABLE.<xmlattr>.xmlns", "http://www.ivoa.net/xml/VOTable/v1.3");
  tree.add ("VOTABLE.<xmlattr>.xmlns:stc",
            "http://www.ivoa.net/xml/STC/v1.30");

  boost::property_tree::ptree &table = tree.add ("VOTABLE.RESOURCE.TABLE", "");
  for (size_t i = 0; i < fields_properties.size (); ++i)
    Field_Properties_to_xml (table, compound_type.getMemberName (i), types[i],
                             fields_properties.at (i));

  boost::property_tree::ptree &tabledata = table.add ("DATA.TABLEDATA", "");
  put_table_in_property_tree (tabledata);
  boost::property_tree::write_xml (
      os, tree, boost::property_tree::xml_writer_settings<char>(' ', 2));
}
