#include <iomanip>

#include "../../Table.hxx"

void Tablator::Table::write_ipac_table_header (std::ostream &os,
                                          const int &num_members) const
{
  os << "\\fixlen = T\n";

  os << std::left;
  const int fits_keyword_length (8);

  for (auto &p : flatten_properties ())
    {
      // FIXME: need to escape the key and value
      os << "\\" << std::setw (fits_keyword_length) << p.first << "= '"
         << p.second << "'\n";
    }

  for (int i = 0; i < num_members; ++i)
    {
      os << "\\ " << compound_type.getMemberName (i);
      auto unit = fields_properties.at (i).attributes.find ("unit");
      if (unit != fields_properties.at (i).attributes.end ())
        {
          os << " (" << unit->second << ")";
        }
      os << "\n";
      for (auto &description : fields_properties.at (i).descriptions)
        os << "\\ ___ " << description.value << "\n";
      // FIXME: Write out description attributes
    }
}
