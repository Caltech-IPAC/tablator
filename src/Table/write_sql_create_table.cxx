#include "../quote_sql_string.hxx"
#include "../Table.hxx"
#include "../Data_Type_to_SQL.hxx"

void
tablator::Table::write_sql_create_table (std::ostream &os,
                                         const std::string &table_name,
                                         const Format::Enums &sql_type,
                                         const std::string &point_column_name,
                                         const std::string &polygon_column_name) const
{
  std::string quoted_table_name (quote_sql_string (table_name, '"',
                                                   Quote_SQL::IF_NEEDED));
  os << "CREATE TABLE " << quoted_table_name << " (\n";
  if (!point_column_name.empty ())
    {
      os << quote_sql_string (boost::to_upper_copy (point_column_name), '"',
                              Quote_SQL::IF_NEEDED)
         << " geography(POINT,0),\n";
    }
  if (!polygon_column_name.empty ())
    {
      os << quote_sql_string (boost::to_upper_copy (polygon_column_name), '"',
                              Quote_SQL::IF_NEEDED)
         << " geography(POLYGON,0),\n";
    }
  for (size_t i = 1; i < columns.size (); ++i)
    {
      os << quote_sql_string (boost::to_upper_copy (columns[i].name), '"',
                              Quote_SQL::IF_NEEDED)
         << " " << Data_Type_to_SQL (columns[i].type, columns[i].array_size,
                                     sql_type);
      if (i + 1 != columns.size ())
        {
          os << ",";
        }
      os << "\n";
    }
  os << ")\n";
}
