#include "../../Table.hxx"

namespace tablator
{
std::list<std::vector<std::string> > parse_dsv
(const boost::filesystem::path &path);

void Table::read_dsv (const boost::filesystem::path &path)
{
  std::list<std::vector<std::string> > dsv (parse_dsv(path));
  set_column_info(dsv);
  read_dsv_rows(dsv);
}
}
