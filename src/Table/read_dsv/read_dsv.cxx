#include "../../Table.hxx"

namespace tablator
{
std::list<std::vector<std::string> >
parse_dsv (std::istream &input_stream, const char &delimiter);

void Table::read_dsv (std::istream &input_stream, const Format &format)
{
  std::list<std::vector<std::string> > dsv (
      parse_dsv (input_stream, format.is_csv () ? ',' : '\t'));
  set_column_info (dsv);
  read_dsv_rows (dsv);
}
}
