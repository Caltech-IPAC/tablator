#include <iomanip>

#include "../../Table.hxx"

void tablator::Table::write_ipac_table_header (std::ostream &os) const
{
  os << "\\fixlen = T\n";

  os << std::left;

  os << "\\RowsRetrieved= " << num_rows () << "\n";

  // FIXME: need to escape the key and value and handle embedded newlines
  const size_t keyword_alignment (8);
  for (auto &property : properties)
    {
      auto &p = property.second;
      if (!p.value.empty ())
        {
          os << "\\" << std::setw (keyword_alignment) << property.first << "= "
             << "'" << p.value << "'\n";
        }
      auto &a = p.attributes;
      auto name (a.find ("name")), value (a.find ("value"));
      if (a.size () == 2 && name != a.end () && value != a.end ())
        {
          os << "\\" << std::setw (keyword_alignment) << name->second << "= "
             << "'" << value->second << "'\n";
        }
      else
        {
          for (auto &attr : a)
            {
              os << "\\" << std::setw (keyword_alignment)
                 << (property.first + "." + attr.first) << "= "
                 << "'" << attr.second << "'\n";
            }
        }
    }

  if (comments.empty ())
    {
      for (size_t i = 1; i < columns.size (); ++i)
        {
          if (!columns[i].field_properties.attributes.empty ()
              || !columns[i].field_properties.description.empty ())
            {
              os << "\\ " << columns[i].name;
              auto unit
                  = columns[i].field_properties.attributes.find ("unit");
              if (unit != columns[i].field_properties.attributes.end ()
                  && !unit->second.empty ())
                {
                  os << " (" << unit->second << ")";
                }
              os << "\n";
              if (!columns[i].field_properties.description.empty ())
                os << "\\ ___ " << columns[i].field_properties.description
                   << "\n";
              // FIXME: Write out description attributes
            }
        }
    }
  else
    {
      for (auto &c : comments)
        {
          std::stringstream ss (c);
          std::string line;
          while (std::getline (ss, line))
            os << "\\ " << line << "\n";
        }
    }
}
