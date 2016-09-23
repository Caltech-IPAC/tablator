#include "../../Table.hxx"

void tablator::Table::read_csv (const boost::filesystem::path &path)
{
  CSV::CSV_Document csv(path.string());
  if (csv.empty ())
    throw std::runtime_error ("This CSV/TSV file is empty: " + path.string());

  set_column_info(csv);
  read_csv_rows(csv);
}
