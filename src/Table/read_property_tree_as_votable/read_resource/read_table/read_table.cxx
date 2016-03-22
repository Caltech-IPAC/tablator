#include "../../../../Table.hxx"
#include "VOTable_Field.hxx"

void tablator::Table::read_table (const boost::property_tree::ptree &table)
{
  auto child = table.begin ();
  auto end = table.end ();

  read_node_and_attributes ("RESOURCE.TABLE", table);
  if (child != end && child->first == "DESCRIPTION")
    {
      properties.emplace_back ("RESOURCE.TABLE.DESCRIPTION",
                               child->second.get_value<std::string>());
      ++child;
    }
  for (; child != end && child->first == "INFO"; ++child)
    {
      read_node_and_attributes ("RESOURCE.TABLE.INFO", child->second);
    }

  std::vector<VOTable_Field> fields;
  fields.emplace_back ("null_bitfield_flag", Data_Type::CHAR, true,
                       Field_Properties (null_bitfield_flags_description, {}));
  for (; child != end; ++child)
    {
      if (child->first == "FIELD")
        {
          fields.emplace_back (read_field (child->second));
        }
      else if (child->first == "PARAM")
        {
          /// PARAM is just metadata and not actually used anywhere
          // read_param (*(child->second))
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
  if (child != end && child->first == "DATA")
    {
      read_data (child->second, fields);
    }
  for (; child != end && child->first == "INFO"; ++child)
    {
      read_node_and_attributes ("RESOURCE.TABLE.INFO", child->second);
    }
}
