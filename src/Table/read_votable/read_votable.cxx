#include <boost/property_tree/xml_parser.hpp>

#include "../../Table.hxx"

/// This only parses VOTable v1.3.

inline void throw_if_empty(const boost::property_tree::ptree::const_iterator &child,
                           const boost::property_tree::ptree::const_iterator &end)
{
  if (child==end)
    throw std::runtime_error ("Missing RESOURCE in VOTABLE");
}

void tablator::Table::read_votable (const boost::filesystem::path &path)
{
  boost::property_tree::ptree tree;
  boost::filesystem::ifstream file(path);
  boost::property_tree::read_xml (file, tree);
  auto votable=tree.get_child ("VOTABLE");
  auto child = votable.begin ();
  auto end = votable.end ();
  throw_if_empty (child, end);

  Property votable_property ("");
  while (child->first == "<xmlattr>")
    {
      for (auto &attribute: child->second)
        if (!(attribute.first == "version"
              || attribute.first == "xmlns:xsi"
              || attribute.first == "xmlns"
              || attribute.first == "xmlns:stc"))
          votable_property.attributes.insert
            (std::make_pair (attribute.first,
                             attribute.second.get_value<std::string> ()));
      ++child;
      throw_if_empty (child, end);
    }      
  if (!votable_property.value.empty () || !votable_property.attributes.empty ())
    properties.emplace_back ("VOTABLE", votable_property);

  if (child->first == "DESCRIPTION")
    {
      comments.emplace_back (child->second.get_value<std::string> ());
      ++child;
      throw_if_empty (child, end);
    }      
  if (child->first == "DEFINITIONS")
    {
      /// Deliberately ignore DEFINITIONS.  They are duplicated by the
      /// information in RESOURCE and deprecated since version 1.1.
      ++child;
      throw_if_empty (child, end);
    }      
  while (child->first != "RESOURCE")
    {
      if (child->first == "COOSYS" || child->first == "PARAM"
          || child->first == "INFO")
        {
          read_node_and_attributes (child);
        }
      else if (child->first == "GROUP")
        {
          // FIXME: Implement groups
        }
      else
        {
          throw std::runtime_error
            ("In VOTABLE, expected COOSYS, GROUP, "
             "PARAM, INFO, or RESOURCE, but got: " + child->first);
        }
      ++child;
      throw_if_empty (child, end);
    }      
  read_resource (child->second);
  ++child;
  if (child != end && child->first == "INFO")
    {
      read_node_and_attributes (child);
      ++child;
    }
  if (child != end)
    throw std::runtime_error ("Unexpected extra node at the end: "
                              + child->first);
}
