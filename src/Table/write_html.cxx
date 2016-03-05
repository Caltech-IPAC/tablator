#include <boost/property_tree/xml_parser.hpp>

#include "../Table.hxx"

void tablator::Table::write_html (std::ostream &os) const
{
  boost::property_tree::ptree tree;

  boost::property_tree::ptree &html = tree.add ("html", "");
  html.add ("head.style", "table,th,td\n{\nborder:1px solid "
                          "black;\nborder-collapse:collapse\n}\n");
  boost::property_tree::ptree &table = html.add ("body.table", "");

  boost::property_tree::ptree &heading_tr = table.add ("TR", "");
  /// skip null_bitfield_flag
  for (size_t i = 1; i < fields_properties.size (); ++i)
    heading_tr.add ("TH", compound_type.getMemberName (i));

  put_table_in_property_tree (table, false);
  os << "<!DOCTYPE HTML>\n";
  // FIXME: This uses the undocumented function write_xml_element
  // since write_xml always writes the <?xml...> header.
  boost::property_tree::xml_parser::write_xml_element (
      os, std::string (), tree, -1,
      boost::property_tree::xml_writer_settings<char>(' ', 2));
}
