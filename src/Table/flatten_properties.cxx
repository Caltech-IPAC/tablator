#include "../Table.hxx"

std::vector<std::pair<std::string, std::string> >
TAP::Table::flatten_properties () const
{
  std::vector<std::pair<std::string, std::string> > result;
  for (auto &p : properties)
    {
      std::vector<std::pair<std::string, std::string> > flatten (
          p.second.flatten (p.first));
      result.insert (result.end (), flatten.begin (), flatten.end ());
    }
  return result;
}
