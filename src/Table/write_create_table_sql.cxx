#include "../quote_sql_string.hxx"
#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"

void
tablator::Table::write_create_table_sql (std::ostream &os,
                                         const std::string &table_name,
                                         const Format::Enums &sql_type) const
{
  std::string quoted_table_name (quote_sql_string (table_name, '"'));
  os << "CREATE TABLE " << quoted_table_name << " (\n";
  for (size_t i = 1; i < columns.size (); ++i)
    {
      os << quote_sql_string (boost::to_upper_copy (columns[i].name), '"')
         << " " << Data_Type_to_SQL (columns[i].type, sql_type);
      if (i + 1 != columns.size ())
        {
          os << ",";
        }
      os << "\n";
    }
  os << ");\n";
}
