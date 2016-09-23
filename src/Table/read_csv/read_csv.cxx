#include "../../Table.hxx"

namespace tablator
{
std::list<std::vector<std::string> > parse_csv
(const boost::filesystem::path &path);

void Table::read_csv (const boost::filesystem::path &path)
{
  std::list<std::vector<std::string> > csv (parse_csv(path));
  set_column_info(csv);
  read_csv_rows(csv);
}
}
