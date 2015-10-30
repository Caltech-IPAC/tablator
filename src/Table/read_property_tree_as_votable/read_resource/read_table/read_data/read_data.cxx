#include <algorithm>

#include "../../../../../Table.hxx"

void tablator::Table::read_data (const boost::property_tree::ptree &data,
                                 const std::vector<std::string> &names)
{
  auto child = data.begin ();
  auto end = data.end ();

  if (child == end)
    throw std::runtime_error ("RESOURCE.TABLE.DATA must not be empty");

  if (child->first == "TABLEDATA")
    {
      read_tabledata(child->second, names);
      ++child;
    }
  else if (child->first == "BINARY")
    {
      throw std::runtime_error ("BINARY serialization inside a VOTable not "
                                "yet supported.");
    }
  else if (child->first == "BINARY2")
    {
      // FIXME: handle BINARY2.  For fixed sizes, it should be a
      // simple memcopy.
      throw std::runtime_error ("BINARY2 serialization inside a VOTable not "
                                "yet supported.");
    }
  else if (child->first == "FITS")
    {
      throw std::runtime_error ("FITS serialization inside a VOTable not "
                                "yet supported.");
    }
  else
    {
      throw std::runtime_error ("Invalid element inside RESOURCE.TABLE.DATA.  "
                                "Expected one of TABLEDATA, BINARY, BINARY2, "
                                "or FITS, but got: " + child->first);
    }
  
  for (; child!=end; ++child)
    {
      if (child->first == "INFO")
        read_node_and_attributes ("RESOURCE.TABLE.DATA.INFO", child->second);
      else
        throw std::runtime_error
          ("Invalid element inside RESOURCE.TABLE.DATA.  "
           "Expected INFO but got: " + child->first);
    }
}
