#include "../../Table.hxx"

namespace tablator
{
std::list<std::vector<std::string> >
parse_dsv (const boost::filesystem::path &path, const char &delimiter);

void Table::read_dsv (const boost::filesystem::path &path,
                      const Format &format)
{
  std::list<std::vector<std::string> > dsv (
      parse_dsv (path, format.is_csv () ? ',' : '\t'));
  set_column_info (dsv);
  read_dsv_rows (dsv);
}
}
