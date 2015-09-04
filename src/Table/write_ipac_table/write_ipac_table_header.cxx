#include <iomanip>

#include "../../Table.hxx"

void tablator::Table::write_ipac_table_header (std::ostream &os,
                                               const int &num_members) const
{
  os << "\\fixlen = T\n";

  os << std::left;

  os << "\\RowsRetrieved= " << size () << "\n";

  for (auto &p : flatten_properties ())
    {
      // FIXME: need to escape the key and value and handle embedded newlines
      const size_t keyword_alignment (8);
      os << "\\" << std::setw (keyword_alignment) << p.first << "= "
         << "'" << p.second << "'\n";
    }

  if (comments.empty () && fields_properties.size () > 0)
    {
      for (int i = 0; i < num_members; ++i)
        {
          os << "\\ " << compound_type.getMemberName (i+1);
          auto unit = fields_properties.at (i+1).attributes.find ("unit");
          if (unit != fields_properties.at (i+1).attributes.end ())
            {
              os << " (" << unit->second << ")";
            }
          os << "\n";
          if (!fields_properties.at (i+1).description.empty ())
            os << "\\ ___ " << fields_properties.at (i+1).description << "\n";
          // FIXME: Write out description attributes
        }
    }
  else
    {
      for (auto &c : comments)
        {
          std::stringstream ss(c);
          std::string line;
          while (std::getline (ss, line))
            os << "\\ " << line << "\n";
        }
    }
}
