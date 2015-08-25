#include <iomanip>

#include "../../Table.hxx"

void tablator::Table::write_ipac_table_header (std::ostream &os,
                                               const int &num_members) const
{
  os << "\\fixlen = T\n";

  os << std::left;

  /// FIXME:: why related to fits?
  const int fits_keyword_length (8);
  os << "\\RowsRetrieved= " << size () << "\n";

  for (auto &p : flatten_properties ())
    {
      // FIXME: need to escape the key and value
      os << "\\" << std::setw (fits_keyword_length) << p.first << "= "
         << "'" << p.second << "'\n";
    }

  if (comments.empty () && fields_properties.size () > 0)
    {
      /// FIXME: Suggest to review this and remove this part
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
