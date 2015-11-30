#include "../../../Table.hxx"

void tablator::Table::read_resource (const boost::property_tree::ptree &resource)
{
  auto child = resource.begin ();
  auto end = resource.end ();

  read_node_and_attributes ("RESOURCE", resource);
  if (child != end && child->first == "DESCRIPTION")
    {
      properties.emplace_back ("RESOURCE.DESCRIPTION",
                               child->second.get_value<std::string> ());
      ++child;
    }      
  for (; child != end && child->first == "INFO"; ++child)
    {
      read_node_and_attributes ("RESOURCE.INFO", child->second);
    }      
  for (; child != end; ++child)
    {
      if (child->first == "COOSYS" || child->first == "PARAM")
        {
          read_node_and_attributes ("RESOURCE." + child->first, child->second);
        }
      else if (child->first == "GROUP")
        {
          // FIXME: Implement groups
        }
      else
        {
          break;
        }
    }      
  /// We only allow one TABLE per RESOURCE
  while (child != end)
    {
      if (child->first == "LINK")
        {
          read_node_and_attributes ("RESOURCE.LINK", child->second);
          ++child;
        }
      if (child != end && child->first == "TABLE")
        {
          if (compound_type.getNmembers ()!=0)
            throw std::runtime_error ("Multiple TABLE elements is not implemented.");
          read_table (child->second);
          ++child;
        }
      if (child != end && child->first == "INFO")
        {
          read_node_and_attributes ("RESOURCE.INFO", child->second);
        }
    }
}
