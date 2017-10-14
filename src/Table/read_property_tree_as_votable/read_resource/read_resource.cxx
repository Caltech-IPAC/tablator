#include "../skip_xml_comments.hxx"
#include "../../../Table.hxx"
#include "VOTable_Field.hxx"

void
tablator::Table::read_resource (const boost::property_tree::ptree &resource)
{
  auto child = resource.begin ();
  auto end = resource.end ();

  read_node_and_attributes ("RESOURCE", resource);
  child=skip_xml_comments(child, end);
  while (child != end && child->first == "<xmlattr>")
    {
      ++child;
      child=skip_xml_comments(child, end);
    }
  if (child != end && child->first == "DESCRIPTION")
    {
      properties.emplace_back ("RESOURCE.DESCRIPTION",
                               child->second.get_value<std::string>());
      ++child;
    }
  child=skip_xml_comments(child, end);
  while (child != end && child->first == "INFO")
    {
      read_node_and_attributes ("RESOURCE.INFO", child->second);
      ++child;
      child=skip_xml_comments(child, end);
    }
  while (child != end)
    {
      if (child->first == "COOSYS")
        {
          read_node_and_attributes ("RESOURCE." + child->first, child->second);
        }
      else if (child->first == "PARAM")
        {
          resource_params.emplace_back (read_field (child->second));
        }
      else if (child->first == "GROUP")
        {
          // FIXME: Implement groups
        }
      else
        {
          break;
        }
      ++child;
      child=skip_xml_comments(child, end);
    }
  /// We only allow one TABLE per RESOURCE
  for (; child != end; ++child)
    {
      if (child->first == "LINK")
        {
          read_node_and_attributes ("RESOURCE.LINK", child->second);
        }
      else if (child->first == "TABLE")
        {
          if (!columns.empty ())
            throw std::runtime_error (
                "Multiple TABLE elements are not implemented.");
          read_table (child->second);
        }
      else if (child->first == "INFO")
        {
          read_node_and_attributes ("RESOURCE.INFO", child->second);
        }
      /// skip <xmlattr> and <xmlcomment>
    }
}
